#include "array.h"

#if defined(MEM_DEBUG)
    #define new 		DEBUG_NEW
    #define delete 	DEBUG_DELETE
#endif

namespace util
{
    array::array(char** pDataAddr,super_block* pObjShm,int nObjectIndex)
    {
        m_nMaxSize = pObjShm->obj_count;
        m_nSize = 0;
        m_nRecordSize = pObjShm->obj_size;
        m_nObjectIndex = nObjectIndex;
        m_nObjBlockNum = pObjShm->obj_blk_num;
        m_nTotalObjBlockNum = pObjShm->obj_total_blk_num;

        for ( int i = 0; i < MAX_MEM_BLKS; i ++ ) {
            if ( i < m_nTotalObjBlockNum )
                m_pData[i] = pDataAddr[i];
            else
                m_pData[i] = 0;
        }
        memcpy((char*)&m_memblk,(char*)&pObjShm->obj_mem,sizeof(struct memblock)*MAX_MEM_BLKS);
    }

    array::~array()
    {
    }

    int array::get_size() const
    {
        return m_nSize;
    }

    int array::get_max_size() const
    {
        return m_nMaxSize;
    }

    void array::set_size(int nSize)
    {
        m_nSize = nSize;
    }

    void array::clear()
    {
        m_nSize = 0;
    }

    int array::get_at(const int nIndex,void*& p)
    {
        if ( nIndex < 0 || nIndex >= m_nMaxSize ) {
            p = 0;
            return -1;
        }

        int blkindex,memblkindex;
        get_block_info(nIndex,memblkindex,blkindex);
        p = (void*)(m_pData[blkindex] + m_memblk[memblkindex].blk_offset + (nIndex - m_memblk[memblkindex].obj_start) * m_nRecordSize);
        return 0;
    }

    int array::swap(const int index_a, const int index_b)
    {
        if (index_a < 0 || index_b < 0 || index_a > m_nMaxSize || index_b > m_nMaxSize) {
            return -1;
        }
        void* p1;
        void* p2;
        char* mem_tmp = new char[m_nRecordSize]; 
        assert(mem_tmp);
        get_at(index_a, p1);
        get_at(index_b, p2);

        memcpy(mem_tmp, p1, m_nRecordSize);
        memcpy(p1, p2, m_nRecordSize);
        memcpy(p2, mem_tmp, m_nRecordSize);
        delete[] mem_tmp;
        return 0;
    }

    int array::get_object_index() const
    {
        return m_nObjectIndex;
    }

    int array::get_block_info(int index,int& memblkindex,int& blkindex)
    {
        if ( index  < 0 || index  >=  m_nMaxSize) {
            ostringstream err_msg;
            err_msg.str("");
            err_msg << "get_block_info: index = " << index <<  " exceed array size = " << m_nMaxSize << endl;
            throw bad_msg(__FILE__,__LINE__,13,err_msg.str());
        }

        if ( m_nObjBlockNum < 1 ) {
            throw bad_msg(__FILE__,__LINE__,14,"get_block_info: object block number < 1.");
        }

        if ( m_nObjBlockNum == 1 ) {
            if ( index > m_memblk[0].obj_end || index <m_memblk[0].obj_start  ) {
                blkindex = -1;
                memblkindex = -1;
                throw bad_msg(__FILE__,__LINE__,15,"get_block_info: index exceed block range.");
            }
            blkindex = m_memblk[0].blk_index;
            memblkindex = 0;  
            return 0;
        }

        int nBegin= 0;
        int nLast  = m_nObjBlockNum - 1;
        register int nMid = 0;

        //ÉýÐòÅÅÁÐ
        while ( nLast >= nBegin ) {
            nMid = nBegin + (( nLast - nBegin ) >> 1);
            if ( index < m_memblk[nMid].obj_start ) {
                nLast = nMid  - 1;
            } else if ( index > m_memblk[nMid].obj_end ) {
                nBegin = nMid  + 1;
            } else {
                memblkindex = nMid;
                blkindex = m_memblk[nMid].blk_index;
                if ( blkindex < 0 || blkindex >= m_nTotalObjBlockNum ) {
                    throw bad_msg(__FILE__,__LINE__,15,"get_block_info: index exceed block range.");
                }
                return 0;
            }
        }
        memblkindex = -1;
        blkindex = -1;
        throw bad_msg(__FILE__,__LINE__,15,"get_block_info: index exceed block range.");
    }

    int array::alloc_record(void*&  obj)
    {
        if ( m_nSize >= m_nMaxSize ) {
            obj = 0;
            throw bad_msg(__FILE__,__LINE__,13,"alloc_record: element count exceed array size.");
        }

        int blkindex,memblkindex;
        get_block_info(m_nSize,memblkindex,blkindex);
        obj = (char*)(m_pData[blkindex] + m_memblk[memblkindex].blk_offset + (m_nSize - m_memblk[memblkindex].obj_start) * m_nRecordSize);
        m_nSize ++;
        return 0;
    }

    int array::calc_hash(enHashKeyMethod nMode,const char* key,int nHashSize)
    {
        int nLen  = 0;
        int  i = 0;
        int  ha = 0;
        //static const unsigned long mask = (unsigned long)0xF0 << (((sizeof(unsigned long) - 1) * 8));
        unsigned long g,hb = 0;

        if ( key == 0 ) {
            throw bad_msg(__FILE__,__LINE__,16,"calc_hash: key is null.");
        }
        switch (nMode) {
        case NUMBER:
            hb = ::atol(key) % nHashSize;
            break;
        case STRING:
            nLen = ::strlen(key);
            for ( i = 0; i < nLen; i ++) {
                if ( key[i] >= '0' && key[i] <= '9' ) {
                    hb = (hb * 64 + key[i] - '0') % nHashSize;
                } else if ( key[i] >= 'A' && key[i] <= 'Z' ) {
                    hb = (hb * 64 + key[i] - 'A' + 10 ) % nHashSize;
                } else if ( key[i] >= 'a' && key[i] <= 'z' ) {
                    hb = (hb * 64 + key[i] - 'a' + 36 ) % nHashSize;
                } else {
                    hb = (hb * 64 + key[i] - ' ' + 32) % nHashSize;
                }

            }

            break;
        }
        ha = (unsigned int)(hb > INT_MAX ? 0 : hb);
        return ha;
    }

    int array::quick_sort(const int left, const int right, icomparer* compar) 
    {
        if (left >= right) {
            return 0;
        }
        int i = left;
        int j = right;
        int pivot = left;
        void* pPivot;
        void* p;
        if (get_at(pivot, pPivot) == -1) {
            return -1;
        }
        while (true) {
            do {
                if (get_at( ++ i, p) == -1) {
                    return -1;
                }
            }while (compar->compare_binary(m_nObjectIndex, p, pPivot) < 0);
            do {
                if (get_at( ++ j, p) == -1) {
                    return -1;
                }
            }while (compar->compare_binary(m_nObjectIndex, p, pPivot) > 0);
            if (i >= j) {
                break;
            }
            swap(i, j);
        }

        swap(j, pivot);
        if (quick_sort(left, j - 1, compar) < 0)
            return -1;
        if (quick_sort(j + 1, right, compar) < 0)
            return -1;

        return 0;

    }
    int array::sort(icomparer* compar)
    {
        return quick_sort(0,m_nSize - 1,compar);
    }

    int array::binary_search(const void * key, icomparer* compar,void*& presult)
    {
        int nBegin= 0;
        int nLast  = m_nSize - 1;
        register int nMid = 0;
        register void* p;

        while ( nLast >= nBegin ) {
            nMid = nBegin + (( nLast - nBegin ) >> 1);        
            if ( get_at(nMid, p) < 0 )
                return -1;

            register int res = compar->compare_binary(m_nObjectIndex,key,p);
            if ( res == 0 ) {
                memcpy((char*)presult,(char*)p,m_nRecordSize);
                return 1;
            }
            //key > p
            if (res > 0)
                nLast = nMid  - 1;
            //key < p
            else
                nBegin = nMid  + 1;
        }
        presult = 0;
        return 0;
    }

    int array::linear_search(const void * key, icomparer* compar,void*& presult)
    {
        int nBegin= 0;
        int nLast  = m_nSize - 1;

        register void* p;
        while ( nLast >= nBegin ) {
            if ( get_at(nBegin, p) < 0 )
                return -1;

            register int res = compar->compare_linear(m_nObjectIndex,key,p);
            if ( res == 0 ) {
                memcpy((char*)presult,(char*)p,m_nRecordSize);
                return 1;
            }
            nBegin ++;
        }
        presult = 0;
        return 0;
    }

}

