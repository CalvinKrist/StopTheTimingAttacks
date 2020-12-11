#include "mem/cache/tags/indexing_policies/fully_associative.hh"

#include "mem/cache/replacement_policies/replaceable_entry.hh"

FullyAssociative::FullyAssociative(const Params* p)
    : BaseIndexingPolicy(p), assoc(p->assoc),
    numSets(1), setShift(0), setMask(0), sets(1),
    tagShift(0){}

uint32_t
FullyAssociative::extractSet(const Addr addr) const{
    return addr;
}

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
SetAssociativeParams::create(){
    return new SetAssociative(this);
}
