// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*
 * HandleFetchSegmentPointer
 *
 * Computes the segment pointer for a given handle.
 *
 */
template<typename TableSegmentHeaderType>
DPTR(TableSegmentHeaderType) HandleFetchSegmentPointerImpl(OBJECTHANDLE handle)
{
    using TableSegmentHeaderPtr = typename DPTR(TableSegmentHeaderType);

    TableSegmentHeaderPtr pSegment = TableSegmentHeaderPtr((uintptr_t)handle & HANDLE_SEGMENT_ALIGN_MASK);
    assert(pSegment);
    return pSegment;
}

/*
 * BlockFetchUserDataPointer
 *
 * Gets the user data pointer for the first handle in a block.
 *
 */
template<typename TableSegmentHeaderType, typename TableSegmentType>
DPTR(uintptr_t) BlockFetchUserDataPointerImpl(DPTR(TableSegmentHeaderType) pSegment, uint32_t uBlock, BOOL fAssertOnError)
{
    // assume NULL until we actually find the data
    DPTR(uintptr_t) pUserData = NULL;
    // get the user data index for this block
    uint32_t blockIndex = pSegment->rgUserData[uBlock];

    // is there user data for the block?
    if (blockIndex != BLOCK_INVALID)
    {
        // In DAC builds, we may not have the entire segment table mapped and in any case it will be quite
        // large. Since we only need one element, we'll retrieve just that one element.  
        pUserData = reinterpret_cast<DPTR(uintptr_t)>(PTR_TO_TADDR(pSegment) + offsetof(TableSegmentType, rgValue) + 
                               (blockIndex * HANDLE_BYTES_PER_BLOCK));
    }
    else if (fAssertOnError)
    {
        // no user data is associated with this block
        //
        // we probably got here for one of the following reasons:
        //  1) an outside caller tried to do a user data operation on an incompatible handle
        //  2) the user data map in the segment is corrupt
        //  3) the global type flags are corrupt
        //
        _ASSERTE(FALSE);
    }

    // return the result
    return pUserData;

}

/*
 * HandleQuickFetchUserDataPointer
 *
 * Gets the user data pointer for a handle.
 * Less validation is performed.
 *
 */
template<typename TableSegmentHeaderType, typename TableSegmentType>
DPTR(uintptr_t) HandleQuickFetchUserDataPointerImpl(OBJECTHANDLE handle)
{
    // get the segment for this handle
    DPTR(TableSegmentHeaderType) pSegment = HandleFetchSegmentPointerImpl<TableSegmentHeaderType>(handle);

    // find the offset of this handle into the segment
    uintptr_t offset = (uintptr_t)handle & HANDLE_SEGMENT_CONTENT_MASK;

    // make sure it is in the handle area and not the header
    assert(offset >= HANDLE_HEADER_SIZE);

    // convert the offset to a handle index
    uint32_t uHandle = (uint32_t)((offset - HANDLE_HEADER_SIZE) / HANDLE_SIZE);

    // compute the block this handle resides in
    uint32_t uBlock = uHandle / HANDLE_HANDLES_PER_BLOCK;

    // fetch the user data for this block
    DPTR(uintptr_t) pUserData = BlockFetchUserDataPointerImpl<TableSegmentHeaderType, TableSegmentType>(pSegment, uBlock, TRUE);

    // if we got the user data block then adjust the pointer to be handle-specific
    if (pUserData)
        pUserData += (uHandle - (uBlock * HANDLE_HANDLES_PER_BLOCK));

    // return the result
    return pUserData;
}

/*
 * HndGetHandleExtraInfo
 *
 * Retrieves owner data from handle.
 *
 */
template<typename TableSegmentHeaderType, typename TableSegmentType>
uintptr_t HndGetHandleExtraInfoImpl(OBJECTHANDLE handle)
{
    // assume zero until we actually get it
    uintptr_t lExtraInfo = 0L;

    // fetch the user data slot for this handle
    DPTR(uintptr_t) pUserData = HandleQuickFetchUserDataPointerImpl<TableSegmentHeaderType, TableSegmentType>(handle);

    // if we did then copy the value
    if (pUserData)
    {
        lExtraInfo = *(pUserData);
    }

    // return the value to our caller
    return lExtraInfo;
}

template<typename TableSegmentHeaderType, typename TableSegmentType>
Object* GetDependentHandleSecondaryImpl(OBJECTHANDLE handle)
{
    return (Object*)HndGetHandleExtraInfoImpl<TableSegmentHeaderType, TableSegmentType>(handle);
}

#ifdef DACCESS_COMPILE
    using TableSegmentType = dac_table_segment;
    using TableSegmentHeaderType = dac_table_segment_header;
#else
    using TableSegmentType = TableSegment;
    using TableSegmentHeaderType = _TableSegmentHeader;
#endif // DACCESS_COMPILE


inline DPTR(TableSegmentHeaderType) HandleFetchSegmentPointer(OBJECTHANDLE handle)
{
    return HandleFetchSegmentPointerImpl<TableSegmentHeaderType>(handle);
}

inline DPTR(uintptr_t) BlockFetchUserDataPointer(DPTR(TableSegmentHeaderType) pSegment, uint32_t uBlock, BOOL fAssertOnError)
{
    return BlockFetchUserDataPointerImpl<
      TableSegmentHeaderType,
      TableSegmentType
    >(
      pSegment,
      uBlock,
      fAssertOnError
     );
}

inline DPTR(uintptr_t) HandleQuickFetchUserDataPointer(OBJECTHANDLE handle)
{
    return HandleQuickFetchUserDataPointerImpl<TableSegmentHeaderType, TableSegmentType>(handle);
}

inline Object* GetDependentHandleSecondary(OBJECTHANDLE handle)
{
    return GetDependentHandleSecondaryImpl<TableSegmentHeaderType, TableSegmentType>(handle);
}
