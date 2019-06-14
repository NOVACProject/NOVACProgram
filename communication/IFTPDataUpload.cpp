#include "StdAfx.h"
#include "IFTPDataUpload.h"
#include "FTPCom.h"
#include "SFTPCom.h"
#include "../Common/Common.h"

namespace Communication
{
    std::unique_ptr<IFTPDataUpload> IFTPDataUpload::Create(const CString& protocol)
    {
        std::unique_ptr<IFTPDataUpload> result;
        if (Equals(protocol, "FTP"))
        {
            result.reset(new CFTPCom());
        }
        else
        {
            result.reset(new CSFTPCom());
        }
        return result;
    }
}