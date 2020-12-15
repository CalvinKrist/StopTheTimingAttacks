#include "instructions.hh"

#include <cstdlib>
#include <stack>
#include <queue>
#include <set>
#include <iostream>
#include <fstream>

#include "mem/cache/base.hh"
#include "mem/xbar.hh"

InstructionFunc instructions[10]{
	inst_CREATETHREAD,
	inst_CREATETHREADWITHSID,
	inst_DELETETHREAD,
	inst_SWITCHTHREAD,
	inst_LOWERSL,
	inst_LOWERNSL,
	inst_RAISESL,
	inst_RAISENSL,
	inst_ATTACH,
    inst_GETLEVEL
};

template<class T, unsigned int C = 8, unsigned int G = 2>
struct inode_list {
    uint32_t count;
    T* directs[C - 2];
    inode_list<T, G* C, G>* indirect;
    T*& operator[](uint32_t idx){
        if(idx >= count){
            panic("Index out of bounds for inode");
        }

        if(idx < C - 2){
            return directs[idx];
        } else{
            return (*indirect)[idx - (C - 2)];
        }
    }

    void push(T* elem){
        if(count < C - 2){
            directs[count++] = elem;
        } else if(count > C - 2){
            indirect->push(elem);
            count++;
        } else{
            indirect = (decltype(indirect)) calloc(1, sizeof(inode_list<T, G* C, G>));
            indirect->push(elem);
            count++;
        }
    }

    T* pop(){
        if(count == C - 1){
            auto elem = indirect->directs[0];
            free(indirect);
            count--;
            return elem;
        } else if(count <= C - 2){
            return directs[--count];
        } else{
	    count--;
            return indirect->pop();
        }
    }

    T* top(){
        return this->operator[](count - 1);
    }

    inode_list() : count{ 0 }{}
};

struct thread_ref;

struct security_level { // 128 bytes
    uint32_t identifier;
    inode_list<thread_ref, 8, 2> threads;
    inode_list<security_level, 15, 2> below;
    inode_list<security_level, 8, 2> above;
    security_level(uint32_t identifier) : identifier{ identifier }{}
}; // 32 per page, 5 bits

struct thread_ref { // 128 bytes
    uint32_t identifier;
    inode_list<security_level, 8, 2> stack;
    inode_list<security_level, 22, 2> references;
    thread_ref(uint32_t identifier) : identifier{ identifier }{}
}; // 32 per page, 5 bits

template<class T>
struct indirect_list {
    std::map<uint32_t, T*> map;

    std::stack<uint32_t> free_list;

    int peak = 1;
    uint32_t new_identifier(){
        if(!free_list.empty()){
            auto tmp = free_list.top();
            free_list.pop();
            map.emplace(tmp, new T(tmp));
            return tmp;
        }

        map.emplace(peak, new T(peak));
        return peak++;
    }

    T* operator[](uint32_t identifier){
        if(map.find(identifier) != map.end()){
            return map.at(identifier);
        } else{
            return nullptr;
        }
    }

    void free_identifier(uint32_t identifier){
        if(map.find(identifier) != map.end()){
            delete map.at(identifier);
            map.erase(identifier);
            free_list.push(identifier);
        }
        //Maybe panic on an else
    }
};

/*template<class T>
struct indirect_list {
    T*** top_level_table;

    uint32_t peak = 1;
    std::stack<uint32_t> free_list;

    T* operator[](uint32_t idx){
        if((idx & 0xF7000000) != 0)
            return nullptr;
        if(top_level_table == nullptr)
            return nullptr;

        auto directory_id = idx >> 15;
        auto entry_id = (idx >> 5) & 0x3FF;
        auto id = idx & 0x1F;

        directory = top_level_table[directory_id];
        if(directory == nullptr)
            return nullptr;

        entry = directory[entry_id];
        if(entry == nullptr)
            return nullptr;

        return &entry[id];
    }

    uint32_t new_identifier(){
        if(!free_list.empty()){
            auto tmp = free_list.top();
            free_list.pop();
            return tmp;
        }

        allocate_new_page(peak);
        return peak++;
    }

    void free_identifier(uint32_t idx){
        if(idx >= peak){
            panic();
        }

        entry = (*this)[idx];
        if(entry == nullptr){
            panic("freeing an invalid identifier");
        }

        //If it's been zeroed out, it isn't currently allocated
        if(*((uint32_t*))entry == 0){
            panic("freeing an unused identifier");
        }

        //Zero it out
        memset(entry, 0, sizeof(T));
        //Add to free list
        free_list.push(idx);
    }

    void allocate_new_page(uint32_t idx){
        if((idx & 0xF7000000) != 0)
            panic("Allocating memory to too high an index!");

        if(top_level_table == nullptr){
            top_level_table = (T***) calloc(4096, 1);
        }

        auto directory_id = idx >> 15;
        auto entry_id = (idx >> 5) & 0x3FF;

        directory = top_level_table[directory_id];
        if(directory == nullptr){
            top_level_table[directory_id] = (T**) calloc(4096, 1);
            directory = top_level_table[directory_id];
        }

        entry = directory[entry_id];
        if(entry == nullptr){
            directory[entry_id] = (T*) calloc(4096, 1);
        }
    }
};*/

typedef indirect_list<security_level> security_list;
typedef indirect_list<thread_ref> thread_list;

security_list slist;
thread_list tlist;

#define MAX_COMPARABLE 512

enum class security_level_comparison {
    same = 0,
    lower = 1,
    higher = 2,
    incomparable = 3
};

void flush_security_cache(INST_COMMON_PARAMS){
    BaseCPU* cpu = context->getCpuPtr();
    auto& port = cpu->getTypedSecPort();
    auto& secCache = (BaseCache&) port.getCache();

    secCache.memInvalidate();
}

#include <iostream>

void add_security_cache_line(INST_COMMON_PARAMS, uint32_t sid, security_level_comparison comparison) {
    BaseCPU * cpu = context->getCpuPtr();
    auto& port = cpu->getTypedSecPort();
    auto& secCache = (BaseCache&)port.getCache();

    auto msid = static_cast<uint32_t>(context->readIntReg(ThreadContext::SID_REG));

    std::cout << "Adding entry to security comparison cache: ";

    if(comparison == security_level_comparison::same){
	    std::cout << sid << " is " << msid << std::endl;
    } else if(comparison == security_level_comparison::lower){
	    std::cout << sid << " is lower than " << msid << std::endl;
    } else {
	    std::cout << sid << " is higher than " << msid << std::endl;
    }
    
    if(!secCache.add_security_cache_line(sid, static_cast <char>(comparison))){
        panic("Unable to add line to security comparison cache!!");
    }
}

void enum_slevel_tree(security_level* level, const std::function<void(security_level*)>& func, bool above){
    std::queue<security_level*> queue{};
    queue.emplace(level);
    std::set<uint32_t> set{};
    while(!queue.empty()){
        auto at = queue.front();
        queue.pop();
        if(above){
            for(uint32_t i = 0; i < at->above.count; i++){
                auto nlevel = at->above[i];
                if(set.find(nlevel->identifier) == set.end()){
                    set.emplace(nlevel->identifier);
                    queue.emplace(nlevel);
                    func(nlevel);
                }
            }
        } else{
            for(uint32_t i = 0; i < at->below.count; i++){
                auto nlevel = at->below[i];
                if(set.find(nlevel->identifier) == set.end()){
                    set.emplace(nlevel->identifier);
                    queue.emplace(nlevel);
                    func(nlevel);
                }
            }
        }
    }
}

void prep_security_cache(INST_COMMON_PARAMS){
    auto level = static_cast<uint32_t>(context->readIntReg(ThreadContext::SID_REG));
    auto active = slist[level];
    if(!active){
        panic("Active level not found");
    }

    flush_security_cache(context);

    int comparable = 0;
    enum_slevel_tree(active, [&comparable, context](security_level* level){
        comparable++;
        if(comparable > MAX_COMPARABLE){
            panic("MAXIMUM COMPARABLE LEVELS EXCEEDED");
        }
        add_security_cache_line(context, level->identifier, security_level_comparison::lower);
    }, false);

    enum_slevel_tree(active, [&comparable, context](security_level* level){
        comparable++;
        if(comparable > MAX_COMPARABLE){
            panic("MAXIMUM COMPARABLE LEVELS EXCEEDED");
        }
        add_security_cache_line(context, level->identifier, security_level_comparison::higher);
    }, true);

    add_security_cache_line(context, active->identifier, security_level_comparison::same);
}

uint32_t create_sid(INST_COMMON_PARAMS){
    auto sid = slist.new_identifier();
    *slist[sid] = security_level(sid);
    return sid;
}

uint32_t create_tid(INST_COMMON_PARAMS){
    auto tid = tlist.new_identifier();
    *tlist[tid] = thread_ref(tid);
    return tid;
}

security_level* lookup_sid(INST_COMMON_PARAMS, uint32_t sid){
    return slist[sid];
}

void set_sid(INST_COMMON_PARAMS, uint32_t sid){
    context->setIntReg(ThreadContext::SID_REG, sid);
    prep_security_cache(context);
}

thread_ref* lookup_tid(INST_COMMON_PARAMS, uint32_t tid){
    return tlist[tid];
}

uint32_t inst_CREATETHREAD(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM){
    return inst_CREATETHREADWITHSID(context, create_sid(context), 0);
}

uint32_t inst_CREATETHREADWITHSID(INST_COMMON_PARAMS, uint32_t SID, UNUSED_INST_PARAM){
    auto thread = lookup_tid(context, create_tid(context));
    auto level = lookup_sid(context, SID);

    enum_slevel_tree(level, [thread, level](security_level* nlevel){
        level->threads.push(thread);
        thread->references.push(nlevel);
    }, false);

    enum_slevel_tree(level, [thread, level](security_level* nlevel){
        level->threads.push(thread);
        thread->references.push(nlevel);
    }, true);

    thread->references.push(level);
    thread->stack.push(level);
    level->threads.push(thread);

    return thread->identifier;
}

uint32_t inst_DELETETHREAD(INST_COMMON_PARAMS, uint32_t TID, UNUSED_INST_PARAM){
    auto thread = lookup_tid(context, TID);
    if(!thread){
        return -1;
    }

    if(TID == context->readIntReg(ThreadContext::TID_REG)){
        return -2;
    }

    for(uint32_t i = 0; i < thread->references.count; i++){
        auto level = thread->references[i];
        for(uint32_t j = 0; j < level->threads.count; j++){
            if(level->threads[j]->identifier == TID){
                if(level->threads.count == 1){
                    slist.free_identifier(level->identifier);
                } else{
                    thread_ref* popped = level->threads.pop();
                    if(j != level->threads.count - 1){
                        level->threads[j] = popped;
                    }
                }
            }
        }
    }

    tlist.free_identifier(TID);
    return 0;
}

uint32_t inst_SWITCHTHREAD(INST_COMMON_PARAMS, uint32_t TID, UNUSED_INST_PARAM){
    auto thread = lookup_tid(context, TID);
    if(!thread){
        return -1;
    }

    context->setIntReg(ThreadContext::TID_REG, TID);
    set_sid(context, thread->stack.top()->identifier);
    return 0;
}

uint32_t inst_LOWERSL(INST_COMMON_PARAMS, uint32_t SID, UNUSED_INST_PARAM){
    auto level  = lookup_sid(context, static_cast<uint32_t>(context->readIntReg(ThreadContext::SID_REG)));
    auto thread = lookup_tid(context, static_cast<uint32_t>(context->readIntReg(ThreadContext::TID_REG)));
    if(!level || !thread){
        return -1;
    }

    bool found = false;
    enum_slevel_tree(level, [&found, SID](security_level* nlevel){
        found = found || nlevel->identifier == SID;
    }, false);

    if(!found){
        return -2;
    }

    auto new_level = lookup_sid(context, SID);
    thread->stack.push(new_level);

    set_sid(context, new_level->identifier);

    return new_level->identifier;
}

uint32_t inst_LOWERNSL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM){
    auto new_level = lookup_sid(context, create_sid(context));
    auto cur_level = lookup_sid(context, static_cast<uint32_t>(context->readIntReg(ThreadContext::SID_REG)));
    auto thread = lookup_tid(context, static_cast<uint32_t>(context->readIntReg(ThreadContext::TID_REG)));
    if(new_level == nullptr || cur_level == nullptr || thread == nullptr){
        return -1;
    }

    cur_level->below.push(new_level);
    new_level->above.push(cur_level);

    for(int i = 0; i < cur_level->threads.count; i++){
        new_level->threads.push(cur_level->threads[i]);
        cur_level->threads[i]->references.push(new_level);
    }

    thread->stack.push(new_level);

    set_sid(context, new_level->identifier);

    return new_level->identifier;
}

uint32_t inst_RAISESL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM){
    auto thread = lookup_tid(context, static_cast<uint32_t>(context->readIntReg(ThreadContext::TID_REG)));
    if(thread == nullptr){
        return -1;
    }

    if(thread->stack.count <= 1){
        return -2;
    }

    thread->stack.pop();

    set_sid(context, thread->stack.top()->identifier);

    return thread->stack.top()->identifier;
}

uint32_t inst_RAISENSL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM){
    auto new_level = lookup_sid(context, create_sid(context));
    auto cur_level = lookup_sid(context, static_cast<uint32_t>(context->readIntReg(ThreadContext::SID_REG)));
    auto thread = lookup_tid(context, static_cast<uint32_t>(context->readIntReg(ThreadContext::TID_REG)));
    if(new_level == nullptr || cur_level == nullptr || thread == nullptr){
        return -1;
    }

    cur_level->above.push(new_level);
    new_level->below.push(cur_level);

    new_level->threads.push(thread);
    thread->references.push(new_level);

    thread->stack.pop();
    thread->stack.push(new_level);

    set_sid(context, new_level->identifier);

    return new_level->identifier;
}

uint32_t inst_ATTACH(INST_COMMON_PARAMS, uint32_t attach_to, uint32_t to_attach){
    auto level = lookup_sid(context, static_cast<uint32_t>(context->readIntReg(ThreadContext::SID_REG)));
    auto attach_to_level = lookup_sid(context, attach_to);
    auto to_attach_level = lookup_sid(context, to_attach);
    if(!level || !attach_to_level || !to_attach_level){
        return -1;
    }

    bool child_found = false;
    enum_slevel_tree(level, [&child_found, to_attach](security_level* nlevel){
        child_found = child_found || nlevel->identifier == to_attach;
    }, false);

    if(!child_found){
        return -2;
    }

    bool parent_found = false;
    enum_slevel_tree(attach_to_level, [&parent_found, to_attach](security_level* nlevel){
        parent_found = parent_found || nlevel->identifier == to_attach;
    }, false);
    enum_slevel_tree(attach_to_level, [&parent_found, to_attach](security_level* nlevel){
        parent_found = parent_found || nlevel->identifier == to_attach;
    }, true);

    if(parent_found){
        return -4;
    }

    attach_to_level->below.push(to_attach_level);
    to_attach_level->above.push(attach_to_level);

    enum_slevel_tree(to_attach_level, [to_attach_level](security_level* nlevel){
        for(int i = 0; i < to_attach_level->threads.count; i++){
            nlevel->threads.push(to_attach_level->threads[i]);
            to_attach_level->threads[i]->references.push(nlevel);
        }
    }, false);

    for(int i = 0; i < attach_to_level->threads.count; i++){
        to_attach_level->threads.push(attach_to_level->threads[i]);
        attach_to_level->threads[i]->references.push(to_attach_level);
    }

    prep_security_cache(context);

    return 0;
}

uint32_t inst_GETLEVEL(INST_COMMON_PARAMS, uint32_t cycles, UNUSED_INST_PARAM){ 
    if(cycles==1) // get the current cycle
        return static_cast<uint32_t>((uint64_t) context->getCpuPtr()->curCycle());
    if(cycles==2) {
        BaseCache* l1Dcache = (BaseCache*)(&((ResponsePort*)(&(context->getCpuPtr()->getDataPort().getPeer())))->owner);
        BaseXBar* toL2Cache = (BaseXBar*)(&((ResponsePort*)(&(l1Dcache->memSidePort.getPeer())))->owner);
        BaseCache* l2Cache = (BaseCache*)(&((ResponsePort*)(&(toL2Cache->memSidePorts.at(0)->getPeer())))->owner);
        //std::cout << l2Cache->name() << std::endl;
        std::ofstream file;
        file.open("occupancy_data.csv", std::ios::app);
        file << l2Cache->linefills << ", ";
        file.close();
        return 0;
    }
	return static_cast<uint32_t>(context->readIntReg(ThreadContext::SID_REG));  // get the current sec_id
}
