#ifndef __SIMPLE_DVB_SI_PARSER_H__
    #define __SIMPLE_DVB_SI_PARSER_H__

#include <cstdio>

/*
 * I am using nullptr and uniform-initialization
 * so minimum C++11 required
 * 
 * Also I am using printf for dump related routines
 * TO DO - use best method for dumping (own logger or cout or ...)
 */

namespace simple_si_parser
{
/*
###############################################
# Basic types that are used by this SI parser #
###############################################
*/
typedef char                SIP_I8;
typedef unsigned char       SIP_UI8;
typedef short int           SIP_I16;
typedef unsigned short int  SIP_UI16;
typedef int                 SIP_I32;
typedef unsigned int        SIP_UI32;

// Basic data class which holds buffer
class CData
{
public:
    CData():m_pui8Data{nullptr},m_ui32DataLen{0}
    {}

    // Set buffer, its size
    inline void SetData(SIP_UI8 * pui8Data, SIP_UI32 ui32DataLen)
    {
        m_pui8Data = pui8Data;
        m_ui32DataLen = ui32DataLen;
    }
    // Get Data Buffer length
    SIP_UI32 GetLen(void) const { return m_ui32DataLen; }
    // Get Data Buffer
    const SIP_UI8 * GetData(void)
    {
        return m_pui8Data;
    }

    // Dump function
    // If invoked without any parameter, it will dump whole data contained by this object
    // Otherwise it will dump (optionally you can use different buffer as well to dump)
    // of passed values
    void HexDump(const SIP_UI8 * pui8Buffer = nullptr, SIP_UI32 ui32Start = 0, SIP_UI32 ui32End = 0)
    {
        if(nullptr == pui8Buffer)
        {
            pui8Buffer = m_pui8Data;
            ui32End = m_ui32DataLen;
        }
        for(SIP_UI32 ui32Loop = ui32Start; ui32Loop < ui32End; ++ui32Loop)
        {
            printf("%02X ", *pui8Buffer++);
            if(((ui32Loop % 16) == 0) && ui32Loop)
            {
                printf("\n");
            }
        }
    }

protected:
    const SIP_UI8 * m_pui8Data;
    SIP_UI32        m_ui32DataLen;
};

// Basic Descriptor
// It assumes that user is provding correct buffer and size, no validation performed
class CDescriptor
{
public:
    CDescriptor(const SIP_UI8 * pui8Data):m_pui8Data{pui8Data}
    {}

    // Get Descriptor Tag
    inline SIP_UI8 GetTag(void)             const { return m_pui8Data[0]; }
    // Get Descriptor Length
    inline SIP_UI8 GetLength(void)          const { return m_pui8Data[1]; }
    // Get Total Descriptor length including descriptor header (1:Tag, 1:Length)
    inline SIP_UI8 GetTotalLength(void)     const { return m_pui8Data[1] + 2; }
    // Get Descriptor Data buffer
    inline const SIP_UI8 * GetData(void)    const { return &m_pui8Data[2]; }
protected:
    const SIP_UI8 * & m_pui8Data;
};

// Basic class for traversing over descriptor list
// It assumes that user is providing correct buffer and size, no validation performed
class CDescriptorList
{
public:
    CDescriptorList(const SIP_UI8 * pui8Data, SIP_UI32 ui32DescListLen):m_pui8Data{pui8Data},m_ui32DescListLen{ui32DescListLen}
    {}

    // Get total descriptor list length
    inline SIP_UI32 GetTotalLength(void) const { return m_ui32DescListLen; }
    // Initialize the list for traversing
    inline void InitList(void) const
    {
        m_pTemp = m_pui8Data;
    }
    // Get next available descriptor in descriptor list
    // if available it will return the descriptor and move to next descriptor
    // otherwise it will return nullptr
    inline const SIP_UI8 * GetDescriptorPtr(void) const
    {
        if((m_pTemp - m_pui8Data) >= m_ui32DescListLen)
            return nullptr;
        
        const SIP_UI8 * pTmp = m_pTemp;
        
        CDescriptor tmp(m_pTemp);
        m_pTemp += tmp.GetTotalLength();

        return pTmp;
    }

protected:
    const SIP_UI8 * &   m_pui8Data;
    SIP_UI32            m_ui32DescListLen;
    mutable const SIP_UI8 * m_pTemp;  // Used only for travesing, not for content modification. So this is mutable
};

}  // namespace simple_si_parser
#endif /* !__SIMPLE_DVB_SI_PARSER_H__ */
