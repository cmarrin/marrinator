/*-------------------------------------------------------------------------
    This source file is a part of m8rscript
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2019, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/

#include "Mallocator.h"

#include "SystemInterface.h"

using namespace m8r;

void* operator new(size_t size)
{
    Mallocator::shared()->init();
    return Mallocator::shared()->allocate<char>(m8r::MemoryType::Fixed, size).get();
}

void operator delete(void* p) noexcept
{
    Mallocator::shared()->deallocate<char>(m8r::MemoryType::Fixed, m8r::Mad<char>(reinterpret_cast<char*>(p)), 0);
}

Mallocator Mallocator::_mallocator;

void Mallocator::init()
{
    if (_heapBase) {
        return;
    }
    
    void* base;
    uint32_t size;
    system()->heapInfo(base, size);
    
    // Limit is one less block, to provide for a NoBlockId value
    if (size < (1024 * 256 - 4)) {
        _memoryInfo.blockSize = 4;
    } else if (size < (1024 * 512 - 8)) {
        _memoryInfo.blockSize = 8;
    } else if (size < (1024 * 1024 - 16)) {
        _memoryInfo.blockSize = 16;
    } else {
        system()->printf(ROMSTR("Exceeded maximum heap size of 1MB\n"));
        return;
    }
    
    _heapBase = reinterpret_cast<char*>(base);
    _memoryInfo.heapSizeInBlocks = static_cast<uint16_t>(size / _memoryInfo.blockSize);
    
    asHeader(0)->nextBlock = NoBlockId;
    asHeader(0)->sizeInBlocks = _memoryInfo.heapSizeInBlocks;
    _firstFreeBlock = 0;
    _memoryInfo.freeSizeInBlocks = _memoryInfo.heapSizeInBlocks;
}

RawMad Mallocator::alloc(size_t size, MemoryType type)
{
    if (!_heapBase) {
        return NoRawMad;
    }
    
    checkConsistency();

    assert(type != MemoryType::Unknown);
    assert(static_cast<uint32_t>(size) <= 0xffff);
    
    uint16_t blocksToAlloc = blockSizeFromByteSize(size);

    // If this is a fixed block we need to remember the size in a 1 block header
    if (type == MemoryType::Fixed) {
        blocksToAlloc++;
    }
    
    BlockId freeBlock = _firstFreeBlock;
    BlockId prevBlock = NoBlockId;
    while (freeBlock != NoBlockId) {
        Header* header = asHeader(freeBlock);
        assert(header->nextBlock != freeBlock);
        
        if (header->sizeInBlocks >= blocksToAlloc) {
            if (blocksToAlloc == header->sizeInBlocks) {
                // Take the whole thing
                if (prevBlock == NoBlockId) {
                    _firstFreeBlock = header->nextBlock;
                } else {
                    asHeader(prevBlock)->nextBlock = header->nextBlock;
                }
                break;
            } else {
                // Take the tail end of the block and resize
                header->sizeInBlocks -= blocksToAlloc;
                freeBlock += header->sizeInBlocks;
                break;
            }
        }
        
        prevBlock = freeBlock;
        freeBlock = header->nextBlock;
    }
    
    if (freeBlock == NoBlockId) {
        return NoRawMad;
    }
    
    RawMad allocatedBlock = freeBlock;
    
    assert(_memoryInfo.numAllocations < std::numeric_limits<uint16_t>::max());
    ++_memoryInfo.numAllocations;
    assert(_memoryInfo.freeSizeInBlocks >= blocksToAlloc);
    _memoryInfo.freeSizeInBlocks -= blocksToAlloc;

    if (allocatedBlock != NoBlockId) {
        if (type == MemoryType::Object) {
            GC::addToStore<MemoryType::Object>(allocatedBlock);
        } else if (type == MemoryType::String) {
            GC::addToStore<MemoryType::String>(allocatedBlock);
        }
        
        if (type == MemoryType::Native) {
            int i = 0;
            (void) i;
        }
        uint32_t index = static_cast<uint32_t>(type);
        
        _memoryInfo.allocationsByType[index].count++;
        _memoryInfo.allocationsByType[index].sizeInBlocks += blocksToAlloc;
        
        if (type == MemoryType::Fixed) {
            asHeader(allocatedBlock)->nextBlock = NoBlockId;
            asHeader(allocatedBlock)->sizeInBlocks = blocksToAlloc;
            allocatedBlock++;
        }
    }

    checkConsistency();
    return allocatedBlock;
}

void Mallocator::coalesce(BlockId prev, BlockId next)
{
    Header* header = asHeader(prev);
    assert(prev + header->sizeInBlocks <= next);
    if (prev + header->sizeInBlocks == next) {
        // Coalesce
        assert(header->nextBlock == next);
        header->nextBlock = asHeader(next)->nextBlock;
        header->sizeInBlocks += asHeader(next)->sizeInBlocks;
    }
}

void Mallocator::free(RawMad p, size_t size, MemoryType type)
{
    if (!_heapBase) {
        return;
    }
    
    checkConsistency();
    assert(type != MemoryType::Unknown);

    if (type == MemoryType::Object) {
        GC::removeFromStore<MemoryType::Object>(p);
    } else if (type == MemoryType::String) {
        GC::removeFromStore<MemoryType::String>(p);
    }
    
    // If this is a fixed block there is a 1 block header
    if (type == MemoryType::Fixed) {
        assert(size == 0);
        size = asHeader(--p)->sizeInBlocks * _memoryInfo.blockSize;
    }
    
    uint16_t blocksToFree = blockSizeFromByteSize(size);

    BlockId newBlock = p;
    Header* newBlockHeader = asHeader(newBlock);
    newBlockHeader->nextBlock = NoBlockId;
    newBlockHeader->sizeInBlocks = blocksToFree;
    
    // Insert in free list
    BlockId freeBlock = _firstFreeBlock;
    BlockId prevBlock = NoBlockId;
    while (freeBlock != NoBlockId) {
        assert(newBlock != freeBlock);
        if (newBlock < freeBlock) {
            if (prevBlock == NoBlockId) {
                // Insert at the head
                newBlockHeader->nextBlock = _firstFreeBlock;
                _firstFreeBlock = newBlock;
            } else {
                newBlockHeader->nextBlock = freeBlock;
                asHeader(prevBlock)->nextBlock = newBlock;
            }
            break;
        }
        prevBlock = freeBlock;
        freeBlock = asHeader(freeBlock)->nextBlock;
    }
    
    if (freeBlock == NoBlockId) {
        // newBlock is beyond end of list. Add it to the tail
        asHeader(prevBlock)->nextBlock = newBlock;
    }
    
    // If this is the only block (super unlikely), no coalescing is needed
    if (asHeader(_firstFreeBlock)->nextBlock != NoBlockId) {
        // 3 cases:
        //
        //      1) prevBlock is NoBlockId:
        //          newBlock is linked into free list at the head
        //          freeBlock is the block after newBlock
        //
        //      2) freeBlock is NoBlockId:
        //          newBlock is linked to the end of the free list
        //          prevBlock is the block before newBlock
        //
        //      3) newBlock is linked to the middle of the free list:
        //          prevBlock is the block before
        //          freeBlock is the block after
        
        if (prevBlock == NoBlockId) {
            assert(asHeader(newBlock)->nextBlock == freeBlock);
            coalesce(newBlock, freeBlock);
        } else if (freeBlock == NoBlockId) {
            assert(asHeader(prevBlock)->nextBlock == newBlock);
            coalesce(prevBlock, newBlock);
        } else {
            assert(asHeader(newBlock)->nextBlock == freeBlock);
            assert(asHeader(prevBlock)->nextBlock == newBlock);
            coalesce(newBlock, freeBlock);
            coalesce(prevBlock, newBlock);
        }
    }

    assert(_memoryInfo.numAllocations > 0);
    --_memoryInfo.numAllocations;
    _memoryInfo.freeSizeInBlocks += blocksToFree;

if (type == MemoryType::Native) {
    int i = 0;
    (void) i;
}
    uint32_t index = static_cast<uint32_t>(type);
    assert(_memoryInfo.allocationsByType[index].count > 0);
    _memoryInfo.allocationsByType[index].count--;
    assert(_memoryInfo.allocationsByType[index].sizeInBlocks >= blocksToFree);
    _memoryInfo.allocationsByType[index].sizeInBlocks -= blocksToFree;
    checkConsistency();
}

ROMString Mallocator::stringFromMemoryType(MemoryType type)
{
    switch(type) {
        case MemoryType::String:        return ROMSTR("String");
        case MemoryType::Character:     return ROMSTR("Char");
        case MemoryType::Object:        return ROMSTR("Object");
        case MemoryType::ExecutionUnit: return ROMSTR("ExecutionUnit");
        case MemoryType::Native:        return ROMSTR("Native");
        case MemoryType::Vector:        return ROMSTR("Vector");
        case MemoryType::UpValue:       return ROMSTR("UpValue");
        case MemoryType::Network:       return ROMSTR("Network");
        case MemoryType::Fixed:         return ROMSTR("Fixed");
        case MemoryType::NumTypes:
        case MemoryType::Unknown:       return ROMSTR("Unknown");
    }
}

void Mallocator::checkConsistency()
{
    for (BlockId block = _firstFreeBlock; block != NoBlockId; block = asHeader(block)->nextBlock) {
        Header* header = asHeader(block);
        assert(header && (header->nextBlock == NoBlockId || block + header->sizeInBlocks < header->nextBlock));
    }
}

// GCC requires the specializations to be in an explicit namespace
namespace m8r {

template<>
Mad<String> Mad<String>::create(const char* s, int32_t length)
{
    Mad<String> obj = Mallocator::shared()->allocate<String>(MemoryType::String, sizeof(String));
    new(obj.get()) String(s, length);
    return obj;
}

template<>
Mad<String> Mad<String>::create(const String& s)
{
    return create(s.c_str());
}

template<>
Mad<String> Mad<String>::create(String&& s)
{
    Mad<String> str = Mad<String>::create();
    *(str.get()) = s;
    return str;
}

}
