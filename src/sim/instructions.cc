#include "instructions.hh"

#include <cstdlib>
#include <stack>
#include <queue>
#include <set>

#include "mem/cache/base.hh"

InstructionFunc instructions[9]{
	inst_CREATETHREAD,
	inst_CREATETHREADWITHSID,
	inst_DELETETHREAD,
	inst_SWITCHTHREAD,
	inst_LOWERSL,
	inst_LOWERNSL,
	inst_RAISESL,
	inst_RAISENSL,
    inst_ATTACH
};

template<class T, unsigned int C = 8, unsigned int G = 2>
struct inode_list {
    uint32_t count;
    T* directs[C - 2];
    inode_list<T, G* C, G>* indirect;
    T operator[](uint32_t idx){
        if(idx >= count){
            panic("Index out of bounds for inode");
        }

        if(idx < C - 2){
            return directs[idx];
        } else{
            return indirect[idx - (C - 2)];
        }
    }

    void push(T* elem){
        if(count < C - 2){
            directs[count++] = elem;
        } else if(count > C - 2){
            indirect->push(elem);
            count++;
        } else{
            indirect = (decltype(indirect)) (calloc(1, sizeof(inode_list<T, G* C, G>));
            indirect->push(elem);
            count++;
        }
    }

    T* pop(){
        if(count == C - 1){
            auto elem = indirect.directs[0];
            free(indirect);
            count--;
            return elem;
        } else if(count <= C - 2){
            return directs[--count];
        } else{
            return indirect->pop();
        }
    }

    T* top(){
        return this->operator[](count - 1);
    }

    inode_list() : count{ 0 }{}
};

struct security_level { // 128 bytes
    uint32_t identifier;
    inode_list<thread_ref> threads;
    inode_list<security_level, 15> below;
    inode_list<security_level> above;
    security_level(uint32_t identifier) : identifier{ identifier }{}
}; // 32 per page, 5 bits

struct thread_ref { // 128 bytes
    uint32_t identifier;
    inode_list<security_level> stack;
    inode_list<security_level, 22> references;
    security_level(uint32_t identifier) : identifier{ identifier }{}
}; // 32 per page, 5 bits

template<class T>
struct indirect_list {
    std::map<uint32_t, T*> map;

    std::stack<uint32_t> free_list;

    uint32_t new_identifier(){
        if(!free_list.empty()){
            auto tmp = free_list.top();
            free_list.pop();
            map.insert(tmp, new T(tmp));
            return tmp;
        }

        map.insert(peak, new T(peak));
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
        }
        free_list.push(identifier);
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
    incomparable = 3;
};

void flush_security_cache(INST_COMMON_PARAMS){
    BaseCache* cache = context->getCpuPtr()->getSecPort()->getCache();

}
void add_security_cache_line(INST_COMMON_PARAMS, uint32_t sid, security_level_comparison comparison){}

void enum_slevel_tree(security_level* level, const std::function<void(security_level*)>& func, bool above){
    std::queue<security_level*> queue{ level };
    std::set<uint32_t> set{};
    while(!queue.empty()){
        auto at = queue.front();
        queue.pop();
        for(uint32_t i = 0; i < above ? at->above.count : at->below.count; i++){
            auto nlevel = above ? at->above[i] : at->below[i];
            if(set.find(nlevel->identifier) == set.end()){
                set.emplace(nlevel->identifier);
                queue.emplace(nlevel);
                func(nlevel);
            }
        }
    }
}

void prep_security_cache(INST_COMMON_PARAMS){
    auto level = static_cast<uint32_t>(context->readIntReg(SID_REG));
    auto active = slist[level];
    if(!active){
        panic("Active level not found");
    }

    flush_security_cache(INST_COMMON_PARAMS);

    int comparable = 0;
    enum_slevel_tree(active, [&comparable](auto level){
        comparable++;
        if(comparable > MAX_COMPARABLE){
            panic("MAXIMUM COMPARABLE LEVELS EXCEEDED");
        }
        add_security_cache_line(context, level->identifier, security_level_comparison::lower);
    }, false);

    enum_slevel_tree(active, [&comparable](auto level){
        comparable++;
        if(comparable > MAX_COMPARABLE){
            panic("MAXIMUM COMPARABLE LEVELS EXCEEDED");
        }
        add_security_cache_line(context, level->identifier, security_level_comparison::higher);
    }, true);

    add_security_cache_line(context, active.identifier, security_level_comparison::same);
}

uint32_t create_sid(INST_COMMON_PARAMS){
    auto sid = slist.new_identifier();
    *slist[sid] = security_level{ sid };
    return sid;
}

uint32_t create_tid(INST_COMMON_PARAMS){
    auto tid = tlist.new_identifier();
    tlist[tid] = thread_ref{ tid };
    return tid;
}

security_level* lookup_sid(INST_COMMON_PARAMS, uint32_t sid){
    return slist[sid];
}

void set_sid(INST_COMMON_PARAMS, uint32_t sid){
    context->setIntReg(SID_REG, sid);
    prep_security_cache(context);
}

thread_ref* lookup_tid(INST_COMMON_PARAMS, uint32_t tid){
    return tlist[tid];
}

uint32_t inst_CREATETHREAD(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM){
    return inst_CREATETHREADWITHSECID(create_sid(context));
}

uint32_t inst_CREATETHREADWITHSID(INST_COMMON_PARAMS, uint32_t SID, UNUSED_INST_PARAM){
    auto thread = lookup_tid(context, create_tid(context));
    auto level = lookup_sid(context, SID);

    enum_slevel_tree(active, [](auto level){
        level->threads.push(thread);
        thread->references.push(nlevel);
    }, false);

    enum_slevel_tree(active, [](auto level){
        level->threads.push(thread);
        thread->references.push(nlevel);
    }, true);

    thread->references.push(level);
    thread->stack.push(level);
    level->threads.push(thread);

    return tid;
}

uint32_t inst_DELETETHREAD(INST_COMMON_PARAMS, uint32_t TID, UNUSED_INST_PARAM){
    auto thread = lookup_tid(context, TID);
    if(!thread){
        return -1;
    }

    if(TID == context->readIntReg(TID_REG)){
        return -2;
    }

    for(uint32_t i = 0; i < thread->references.count; i++){
        auto level = thread->references[i];
        for(uint32_t j = 0; j < level->threads.count; j++){
            if(level->threads[j].identifier == tid){
                if(level->threads[j].count == 1){
                    slist.free_identifier(level->identifier);
                } else{
                    auto popped = level->threads.pop();
                    if(j != level->threads[j].count - 1){
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

    context->setIntReg(TID_REG, TID);
    set_sid(context, thread->stack.top()->identifier);
    return 0;
}

uint32_t inst_LOWERSL(INST_COMMON_PARAMS, uint32_t SID, UNUSED_INST_PARAM){
    auto level  = lookup_sid(context, static_cast<uint32_t>(context->readIntReg(SID_REG)));
    auto thread = lookup_tid(context, static_cast<uint32_t>(context->readIntReg(TID_REG)));
    if(!level || !thread){
        return -1;
    }

    bool found = false;
    enum_slevel_tree(level, [&found, SID](auto nlevel){
        found ||= nlevel->identifier == SID;
    }, false);

    if(!found){
        return -2;
    }

    auto new_level = lookup_sid(context, sid);
    thread->stack.push(new_level);

    set_sid(context, new_level->identifier);

    return new_level->identifier;
}

uint32_t inst_LOWERNSL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM){
    auto new_level = lookup_sid(context, create_sid(context));
    auto cur_level = lookup_sid(context, static_cast<uint32_t>(context->readIntReg(SID_REG)));
    auto thread = lookup_tid(context, static_cast<uint32_t>(context->readIntReg(TID_REG)));
    if(new_level == nullptr || cur_level == nullptr || thread == nullptr){
        return -1;
    }

    cur_level->below.push(new_level);
    new_level->above.push(current_level);
    new_level->threads.push(thread);
    thread->referenes.push(new_level);

    thread->stack.push(new_level);

    set_sid(context, new_level->identifier);

    return new_level->identifier;
}

uint32_t inst_RAISESL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM){
    auto thread = lookup_tid(context, static_cast<uint32_t>(context->readIntReg(TID_REG)));
    if(thread == nullptr){
        return -1;
    }

    if(thread->stack.count == 0){
        return -2;
    }

    thread->stack.pop();

    set_sid(context, thread->stack.top()->identifier);

    return thread->stack.top()->identifier;
}

uint32_t inst_RAISENSL(INST_COMMON_PARAMS, UNUSED_INST_PARAM, UNUSED_INST_PARAM){
    auto new_level = lookup_sid(context, create_sid(context));
    auto cur_level = lookup_sid(context, static_cast<uint32_t>(context->readIntReg(SID_REG)));
    auto thread = lookup_tid(context, static_cast<uint32_t>(context->readIntReg(TID_REG)));
    if(new_level == nullptr || cur_level == nullptr || thread == nullptr){
        return -1;
    }

    cur_level->above.push(new_level);
    new_level->below.push(current_level);
    new_level->threads.push(thread);
    thread->referenes.push(new_level);

    thread->stack.pop();
    thread->stack.push(new_level);

    set_sid(context, new_level->identifier);

    return new_level->identifier;
}

uint32_t inst_ATTACH(INST_COMMON_PARAMS, uint32_t attach_to, uint32_t to_attach){
    auto level = lookup_sid(context, static_cast<uint32_t>(context->readIntReg(SID_REG)));
    auto attach_to_level = lookup_sid(context, attach_to);
    auto to_attach_level = lookup_sid(context, to_attach);
    if(!level || !attach_to_level || !to_attach_level){
        return -1;
    }

    bool child_found = false;
    enum_slevel_tree(level, [&found, to_attach](auto nlevel){
        child_found || = nlevel->identifier == to_attach;
    }, false);

    if(!child_found){
        return -2;
    }

    bool parent_found = false;
    enum_slevel_tree(attach_to_level, [&found, attach_to](auto nlevel){
        parent_found || = nlevel->identifier == attach_to;
    }, false);
    enum_slevel_tree(attach_to_level, [&found, attach_to](auto nlevel){
        parent_found || = nlevel->identifier == attach_to;
    }, false);

    if(parent_found){
        return -4;
    }

    attach_to_level->below.push(to_attach_level);
    to_attach_level->above.push(attach_to_level);

    prep_security_cache(context);

    return 0;
}