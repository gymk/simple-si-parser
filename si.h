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

#define SIP_MAX_MPEG2_PRIVATE_SECTION_SIZE          0x1000  // 4 KiB
#define SIP_MAX_MPEG2_PSI_SECTION_SIZE              0x400   // 1 KiB

#define SIP_PAT_TABLE_ID                            0
#define SIP_CAT_TABLE_ID                            1
#define SIP_PMT_TABLE_ID                            2

// TO DO
static SIP_UI32 SIP_GetMPEG2CRC32(const SIP_UI8 * pui8Buff, SIP_UI32 ui32DaaLen)
{
    return 0xDEADBEEF;
}

// Basic data class which holds buffer
class CData
{
public:
    CData():m_pui8Data{nullptr},m_ui32DataLen{0}
    {}
    CData(const SIP_UI8 * pui8Data, SIP_UI32 ui32DataLen):m_pui8Data{pui8Data},m_ui32DataLen{ui32DataLen}
    {}

    // Set buffer, its size
    inline void SetData(const SIP_UI8 * pui8Data, SIP_UI32 ui32DataLen)
    {
        m_pui8Data = pui8Data;
        m_ui32DataLen = ui32DataLen;
    }
    // Get Data Buffer length
    SIP_UI32 GetLen(void) const { return m_ui32DataLen; }
    // Get Data Buffer
    const SIP_UI8 * GetData(void) const
    {
        return m_pui8Data;
    }

    // Dump function
    // If invoked without any parameter, it will dump whole data contained by this object
    // Otherwise it will dump (optionally you can use different buffer as well to dump)
    // of passed values
    void HexDump(const SIP_UI8 * pui8Buffer = nullptr, SIP_UI32 ui32Start = 0, SIP_UI32 ui32End = 0) const
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
    explicit CDescriptor(const SIP_UI8 * pui8Data):m_pui8Data{pui8Data}
    {}

    // Get Descriptor Tag
    inline SIP_UI8 GetTag(void)             const { return m_pui8Data[0]; }
    // Get Descriptor Length
    inline SIP_UI8 GetLength(void)          const { return m_pui8Data[1]; }
    // Get Total Descriptor length including descriptor header (1:Tag, 1:Length)
    inline SIP_UI8 GetTotalLength(void)     const { return m_pui8Data[1] + 2; }
    // Get Descriptor Data buffer
    inline const SIP_UI8 * GetData(void)    const { return &m_pui8Data[2]; }

    // Dump Function
    void PrintDescriptor(const SIP_I8 * pi8Prefix, bool bDumpContent = false) const
    {
        printf("%s Tag[%d] Len[%d]", pi8Prefix, GetTag(), GetLength());
        if(bDumpContent)
        {
            // TO DO - Add dumping
        }
        printf("\n");
    }
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

class CPrivateSection : public CData
{
public:
    CPrivateSection():CData()
    {}
    CPrivateSection(const SIP_UI8 * pui8Buff, SIP_UI16 ui16SecLen):CData(pui8Buff, ui16SecLen)
    {}

    inline SIP_UI8  GetTableID(void)              const { return m_pui8Data[0]; }
    inline SIP_UI8  GetSSI(void)                  const { return (m_pui8Data[1 & 0x80]); }
    inline SIP_UI16 GetSectionLength(void)        const { return (((m_pui8Data[1] & 0x0F) << 8) | m_pui8Data[2]); }
    inline SIP_UI16 GetTableIdExtension(void)     const { return ((m_pui8Data[3] << 8) | m_pui8Data[4]); }
    inline SIP_UI8  GetVersionNumber(void)        const { return ((m_pui8Data[5] & 0x3E) >> 1); }
    inline SIP_UI8  GetCNI(void)                  const { return (m_pui8Data[5] & 0x1); }
    inline SIP_UI8  GetSectionNumber(void)        const { return (m_pui8Data[6]); }
    inline SIP_UI8  GetLastSectionNumber(void)    const { return (m_pui8Data[7]); }
    inline const SIP_UI8* GetDescriptorList(void) const { return (&m_pui8Data[8]); };
    // Get Private Data - skipping private section header
    inline const SIP_UI8* GetPrivateData(void)    const
    {
        if(GetSSI())
        {
            return &m_pui8Data[8];
        }
        return &m_pui8Data[3];
    }
    // Get Private Section length
    inline SIP_UI16 GetPrivateDataLength(void)    const
    {
        if(GetSSI())
        {
            return GetSectionLength() - 9;  // 4 byte CRC, 5 byte sec header
        }
        return GetSectionLength();
    }
    // returns CRC found in section (if available, otherwise 0xDEADBEEF)
    inline SIP_UI32 GetSectionCRC(void)           const
    {
        SIP_UI16 ui16SecLen = GetSectionLength();

        if(ui16SecLen < m_ui32DataLen)
        {
            return (
                ((m_pui8Data[3 + ui16SecLen - 4]) << 24)
                |
                ((m_pui8Data[3 + ui16SecLen - 3]) << 16)
                |
                ((m_pui8Data[3 + ui16SecLen - 2]) << 8)
                |
                (m_pui8Data[3 + ui16SecLen - 1])
            );
        }
        return 0xDEADBEEF;
    }
    // returns CRC calculated over this section excluding actual crc32 encoded in this section
    inline SIP_UI32 GetCalculatedCRC(void)  const
    {
        return SIP_GetMPEG2CRC32(m_pui8Data, (m_ui32DataLen - 4));
    }
    // returns CRC calcuated over the whole section
    inline SIP_UI32 GetFullDataCRC(void)    const
    {
        return SIP_GetMPEG2CRC32(m_pui8Data, m_ui32DataLen);
    }
    // validates section crc with calcuated crc
    bool ValidateCRC(void)                  const
    {
        return(GetSectionCRC() == GetCalculatedCRC());
    }
    // validates whether this is a valid private section
    bool isSectionValid(void)               const
    {
        return (
            ((GetSectionLength() + 3) == m_ui32DataLen)
            &&
            (GetSectionLength() <= (SIP_MAX_MPEG2_PRIVATE_SECTION_SIZE))
            //&& (GetSSI() ? (GetSectionCRC() == GetCalculatedCRC()) : true)
        );
    }

    // Dump Function
    void PrintSectionInfo(const char * pi8Prefix) const
    {
        if(GetSSI)
        {
            SIP_UI32 secCRC = GetSectionCRC();
            SIP_UI32 calcCRC = GetCalculatedCRC();
            printf("%s TableID[%d] Ver[%d] SecLen[%d] SecCRC[%X] CalcCRC[%X] Result - %s\n",
                pi8Prefix,
                GetTableID(),
                GetVersionNumber(),
                GetSectionLength(),
                secCRC,
                calcCRC,
                ((secCRC == calcCRC) ? "CRC Mattched": "CRC Mismatch found")
                );
        }
        else
        {
            printf("%s TableID[%d] Ver[%d] SecLen[%d] SSI[%d]\n",
                pi8Prefix,
                GetTableID(),
                GetVersionNumber(),
                GetSectionLength(),
                GetSSI()
                );
        }
    }
};

// PAT Section
class CPatSection : public CPrivateSection
{
public:
    CPatSection(const SIP_UI8 * pui8Buff, SIP_UI16 ui16SecLen):CPrivateSection(pui8Buff, ui16SecLen)
    {}

    // Validates PAT section
    inline bool isValidPAT(void) const
    {
        return (
                (m_pui8Data[0] == SIP_PAT_TABLE_ID)
                &&
                GetSSI()
                &&
                isSectionValid()
                &&
                (GetSectionLength() <= (SIP_MAX_MPEG2_PSI_SECTION_SIZE - 3))
        );
    }
    // Get Transport Stream ID of this PAT
    inline SIP_UI16 GetTransportStreamID(void)   const { return GetTableIdExtension(); }
    // Get no. of programs listed by this PAT
    inline SIP_UI8  GetNoOfProgramsInList(void)  const { return ((GetSectionLength() - 9) / 4); }
    
    // Initialize ProgramList for travesing in PAT
    inline void InitProgList(void)               const { m_pTemp = GetPrivateData(); }
    // Get Program Number, its PMT PID of current program in list
    inline void GetProgramInfo(SIP_UI16 * pui16ProgramNo, SIP_UI16 * pui16PmtPid) const
    {
        *pui16ProgramNo = ((m_pTemp[0] << 8) | m_pTemp[1]);
        *pui16PmtPid = (((m_pTemp[2] & 0x1F) << 8) | (m_pTemp[3]));
    }
    // Move to next program info in list, returns true if moved
    // if no more programs available it will return false
    inline bool MoveToNextProgramInfo(void)     const
    {
        m_pTemp += 4;

        if((m_pTemp - m_pui8Data) < GetPrivateDataLength())
        {
            return true;
        }

        return false;
    }

    // Dump Function
    void PrintPAT(void)                         const
    {
        printf("####PAT Dump Start\n");
        if(isValidPAT())
        {
            PrintSectionInfo("####PAT");
            
            SIP_UI16 ui16NoOfPrograms = GetNoOfProgramsInList();
            SIP_UI16 ui16ProgramNo;
            SIP_UI16 ui16ProgramPMTPid;
            SIP_UI16 ui16Count = 0;

            printf("####PAT> No. of programs: %d\n", ui16NoOfPrograms);
            InitProgList();
            do
            {
                GetProgramInfo(&ui16ProgramNo, &ui16ProgramPMTPid);
                printf("%d: ProgramNo[%d,%d]\n", ++ui16Count, ui16ProgramNo, ui16ProgramPMTPid);
            } while(MoveToNextProgramInfo());
        }
        else
        {
            printf("Invalid PAT Found...\n");
            HexDump();
        }
        printf("####PAT Dump End\n");
    }

private:
    mutable const SIP_UI8 * m_pTemp;
};

// PMT Section - TO DO
class CPmtSection : public CPrivateSection
{
public:
    CPmtSection(const SIP_UI8 * pui8Buff, SIP_UI16 ui16SecLen):CPrivateSection(pui8Buff, ui16SecLen)
    {}

    // Validates PMT section
    inline bool isValidPMT(void) const
    {
        return (
                (m_pui8Data[0] == SIP_PMT_TABLE_ID)
                &&
                GetSSI()
                &&
                isSectionValid()
                &&
                (GetSectionLength() <= (SIP_MAX_MPEG2_PSI_SECTION_SIZE - 3))
        );
    }

    // Get this PMT's Program Number
    inline SIP_UI16 GetProgramNumber(void)              const { return GetTableIdExtension(); }
    // Get PCR Pid of this program
    inline SIP_UI16 GetPcrPid(void)                     const { return (((m_pui8Data[8] & 0x1F) << 8) | (m_pui8Data[9])); }
    // Get Program Info descriptor list length
    inline SIP_UI16 GetProgramInfoLength(void)          const { return (((m_pui8Data[10] & 0x0F) << 8) | (m_pui8Data[11])); }
    // Get Program Info descriptor list buffer
    inline const SIP_UI8 * GetProgramInfoDescBuf(void)  const { return (&m_pui8Data[12]); }
    // Get Stream Info list length
    inline SIP_UI16 GetStreamInfoLength(void)           const { return ((GetSectionLength() - 13) - GetProgramInfoLength()); }
    // Get Stream info list buffer
    inline const SIP_UI8 * GetStreamInfoBuffer(void)    const { return (GetProgramInfoDescBuf() + GetProgramInfoLength()); }

    // Traversing Stream Info list
    inline void InitStreamInfoList(void)                const
    {
        m_pTemp = GetStreamInfoBuffer();
    }
    // Get Current Stream Type
    inline SIP_UI8 GetStreamType(void)                  const { return (m_pTemp[0]); }
    // Get Current Stream PID
    inline SIP_UI16 GetStreamPid(void)                  const { return (((m_pTemp[1] & 0x1F) << 8) | m_pTemp[2]); }
    // Get Current Stream's: Elemenentary Stream info length (ES Info descriptor list's length)
    inline SIP_UI16 GetESInfoLength(void)               const { return (((m_pTemp[3] & 0xF) << 8) | m_pTemp[4]); }
    // Get Current Stream's: Elemenentary Stream info buffer (ES Info descriptor list's buffer)
    inline const SIP_UI8 * GetESInfoDescBuff(void)      const { return (m_pTemp + 5); }
    // Move to next Stream Info if exist, return true. Otherwise return false
    inline bool MoveToNextStreamInfoInList(void)        const
    {
        m_pTemp += (GetESInfoLength() + 5);
        if((m_pTemp - GetStreamInfoBuffer()) < GetStreamInfoLength())
        {
            return true;
        }
        return false;
    }

    // Dump Function
    void PrintPMT(void)                                 const
    {
        printf("####PMT Dump Start\n");
        if(isValidPMT())
        {
            PrintSectionInfo("####PMT>");

            // Print Program Info
            printf("ProgInfoLen: %d\n", GetProgramInfoLength());
            if(GetProgramInfoLength())
            {
                printf("####PMT> Program Info Dump Start\n");
                const SIP_UI8 * pui8Buff;
                CDescriptorList descList(GetProgramInfoDescBuf(), GetProgramInfoLength());

                descList.InitList();
                while((pui8Buff = descList.GetDescriptorPtr()))
                {
                    //CDescriptor desc(pui8Buff);
                    //desc.PrintDescriptor("####CAT>");

                    CDescriptor(pui8Buff).PrintDescriptor("####PMT>");
                }
                printf("####PMT> Program Info Dump Start\n");
            }
            // Print Elementary Stream Info
            printf("ESInfoLen: %d\n", GetStreamInfoLength());
            if(GetStreamInfoLength())
            {
                printf("####PMT> Stream Info Dump Start\n");
                InitStreamInfoList();
                do
                {
                    printf("####PMT> StreamType[0x%02X] Pid[0x%04X] EsInfoLen[%d] \n", GetStreamType(), GetStreamPid(), GetESInfoLength());
                    // ES Info
                    if(GetESInfoLength())
                    {
                        printf("####PMT> ES Info Dump Start\n");
                        const SIP_UI8 * pui8Buff;
                        CDescriptorList descList(GetESInfoDescBuff(), GetESInfoLength());

                        descList.InitList();
                        while((pui8Buff = descList.GetDescriptorPtr()))
                        {
                            //CDescriptor desc(pui8Buff);
                            //desc.PrintDescriptor("####CAT>");

                            CDescriptor(pui8Buff).PrintDescriptor("####PMT>");
                        }
                        printf("####PMT> ES Info Dump End\n");
                    }
                }
                while(MoveToNextStreamInfoInList());
                printf("####PMT> Stream Info Dump End\n");
            }
        }
        else
        {
            printf("Invalid PMT Found...\n");
            HexDump();
        }
        printf("####PMT Dump End\n");
    }

private:
    mutable const SIP_UI8 * m_pTemp;
};

// CAT Section
class CCatSection : public CPrivateSection
{
public:
    CCatSection(const SIP_UI8 * pui8Buff, SIP_UI16 ui16SecLen):CPrivateSection(pui8Buff, ui16SecLen)
    {}

    // Validates CAT section
    inline bool isValidCAT(void) const
    {
        return (
                (m_pui8Data[0] == SIP_CAT_TABLE_ID)
                &&
                GetSSI()
                &&
                isSectionValid()
                &&
                (GetSectionLength() <= (SIP_MAX_MPEG2_PSI_SECTION_SIZE - 3))
        );
    }

    // To get CAT Descriptor list buffer pointer
    inline const SIP_UI8 * GetDescriptorListBuffer(void)        const { return GetPrivateData(); }
    // To get CAT Descriptor list length in bytes
    inline SIP_UI16        GetDescriptorListBufferLength(void)  const { return GetPrivateDataLength(); }

    // Dump Function
    void PrintCAT(void) const
    {
        printf("####CAT Dump Start\n");
        if(isValidCAT())
        {
            PrintSectionInfo("####CAT");
            printf("CAT Descriptor list length: %d\n", GetDescriptorListBufferLength());

            if(GetDescriptorListBufferLength())
            {
                const SIP_UI8 * pui8Buff;
                CDescriptorList descList(GetDescriptorListBuffer(), GetDescriptorListBufferLength());

                descList.InitList();
                while((pui8Buff = descList.GetDescriptorPtr()))
                {
                    //CDescriptor desc(pui8Buff);
                    //desc.PrintDescriptor("####CAT>");

                    CDescriptor(pui8Buff).PrintDescriptor("####CAT>");
                }
            }
        }
        else
        {
            printf("Invalid CAT Found...\n");
            HexDump();
        }
        printf("####CAT Dump End\n");
    }
};

// ECM Section
class CEcmSection : public CPrivateSection
{
public:
    CEcmSection(const SIP_UI8 * pui8Buff, SIP_UI16 ui16SecLen):CPrivateSection(pui8Buff, ui16SecLen)
    {}

    // Validates ECM section
    inline bool isValidECM(void) const
    {
        if(GetSSI())
        {
            return (isSectionValid() && ValidateCRC());
        }
        
        return isSectionValid();
    }

    // To get ECM Data
    inline const SIP_UI8 * GetEcmDataBuffer(void)       const { return GetPrivateData(); }
    // To get ECM Data Length
    inline SIP_UI16        GetEcmDataBufferLength(void) const { return GetPrivateDataLength(); }

    // Dump Function
    void PrintECM(void) const
    {
        printf("####ECM Dump Start\n");
        if(isValidECM())
        {
            PrintSectionInfo("####ECM");
            printf("ECM Data Length: %d\n", GetEcmDataBufferLength());
            HexDump(GetEcmDataBuffer(), 0, GetEcmDataBufferLength());
        }
        else
        {
            printf("Invalid ECM Found...\n");
            HexDump();
        }
        printf("####ECM Dump End\n");
    }
};

// NIT Section - TO DO
// EMM Section - TO DO

}  // namespace simple_si_parser
#endif /* !__SIMPLE_DVB_SI_PARSER_H__ */
