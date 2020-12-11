#include "mem/cache/tags/indexing_policies/fully_associative.hh"

#include "mem/cache/replacement_policies/replaceable_entry.hh"

const FullyAssociative::Params* fixParams(const FullyAssociative::Params* p){
    auto tmp = const_cast<FullyAssociative::Params*>(p);
    tmp->entry_size = 1;
    tmp->size = p->assoc;
    return p;    
}

FullyAssociative::FullyAssociative(const Params* p)
    : BaseIndexingPolicy(fixParams(p)){}

Addr
FullyAssociative::regenerateAddr(const Addr tag, const ReplaceableEntry* entry)
const{
    return tag;
}

std::vector<ReplaceableEntry*>
FullyAssociative::getPossibleEntries(const Addr addr) const{
    return sets[0];
}

FullyAssociative*
FullyAssociativeParams::create(){
    return new FullyAssociative(this);
}
