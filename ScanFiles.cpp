/*!
*  @brief     磁盘文件信息扫描
*  @details   获取当前操作系统的磁盘文件信息，以tcp客户端形式，将数据发送至服务器。
*  @author    wey
*  @version   1.0
*  @date      2019.08.22
*  @warning
*  @copyright NanJing RenGu.
*  @note
*/
#include "Header.h"
#include "md5.h"
#include "MyLog.h"
#include "RUtil.h"
#include "global.h"
#include "socket.h"

char* UnicodeToUTF8(const char* src, int srclen, int &len);

MyLog m_log;

// 根据scanPath路径，通过递归扫描类型为scanType的文件，并放于fileList。
void ScanFiles(char *scanPath, char *scanType, vector<Stru_ScanRets> &fileList) {
#ifdef WIN32
    char dirNew[300];
    memset(dirNew, 0, 300);
    strcpy(dirNew, scanPath);
    strcat(dirNew, "/*.*");    // 获得该路径下所有的文件

    _finddata_t findData;
    intptr_t handle;
    handle = _findfirst(dirNew, &findData);
    if (handle == -1)
    {
        char buffer[40];
        sprintf(buffer, "open dir %s failed\n", dirNew);
        //writeLog(buffer);
        return;
    }


    do
    {
        if (findData.attrib & _A_SUBDIR) // 判断该文件是否为文件夹
        {
            if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
                continue;

            // 组成新的文件路径，进行扫描
            strcpy(dirNew, scanPath);
            strcat(dirNew, "/");
            strcat(dirNew, findData.name);

            ScanFiles(dirNew, scanType, fileList);
        }
        else
        {
            // 当前为文件
            if ((strcmp(scanType, "") == 0) || m_log.decFileName(findData.name, scanType) == true)
            {
                Stru_ScanRets scanRets;
                char utfPath[300];
                memset(utfPath, 0, 300);
                strcat(utfPath, scanPath);
                strcat(utfPath, "/");
                strcat(utfPath, findData.name);
                int utflens = 0;

//                char * temppath = (char*)UnicodeToUTF8(utfPath, strlen(utfPath), utflens);
//                memcpy(scanRets.filePath, temppath, utflens);

				memcpy(scanRets.filePath, utfPath,strlen(utfPath));

                // 计算该文件的MD5值
                MD5_CTX mdvalue;
                mdvalue.GetFileMd5(scanRets.md5Value, utfPath);
//				RUtil::printError("File md5:[%s][%s]", utfPath, scanRets.md5Value);

                // 将扫描结果放入队列中
                fileList.push_back(scanRets);
            }

        }
    } while (_findnext(handle, &findData) == 0);

    _findclose(handle);
#else
    char dirNew[300] = {0};
    strcpy(dirNew, scanPath);
    DIR *pDir;
    pDir = opendir(dirNew);
    if (pDir == NULL) {
        RUtil::printError("open dir %s failed",dirNew);
        return;
    }
    struct dirent *pDirEnt = NULL;

    while ((pDirEnt = readdir(pDir)) != NULL) {
        if ((strcmp(pDirEnt->d_name, ".") == 0) || (strcmp(pDirEnt->d_name, "..") == 0))
            continue;

        struct stat fileStat;
        char filePath[256] = {0};                    //文件路径
        strcat(filePath, scanPath);
        strcat(filePath, "/");
        strcat(filePath, pDirEnt->d_name);
        // 获得文件属性
        stat(filePath, &fileStat);

        // 判断是否为文件夹
        if (S_ISDIR(fileStat.st_mode))
        {
            ScanFiles(filePath, scanType, fileList);
        } else {
            // 当前为文件
            if ((strcmp(scanType, "") == 0) || m_log.decFileName(pDirEnt->d_name, scanType)) {
                Stru_ScanRets scanRets;
                strcat(scanRets.filePath, filePath);
                // 计算该文件的MD5值
                MD5_CTX mdvalue;
                mdvalue.GetFileMd5(scanRets.md5Value, scanRets.filePath);
                RUtil::printError("File md5:[%s][%s]",filePath,scanRets.md5Value);
                // 将扫描结果放入队列中
                fileList.push_back(scanRets);
            }
        }
    }
    closedir(pDir);
#endif
}


// 通过tcp返回扫描结果
void SendScanRets(Stru_ScanRetReco retReco)
{
    RSocket tclient;
    if (!tclient.createSocket(RSocket::R_TCP)){
        RUtil::printError("create scan file socket error!");
        return;
    }

    if (g_progFlag)
    {
		if (!G_ServerIP.empty())
        {
			if (tclient.connect(G_ServerIP.c_str(), SCANRETS)){

                int len = sizeof(retReco)+retReco.fileNum * (256 + 34);

                char *sendbuf = new char[len];
                memset(sendbuf, 0, len);

                int index = 0;
                memcpy(sendbuf, retReco.sign, sizeof(retReco.sign));
                index += sizeof(retReco.sign);

                memcpy(sendbuf + index, retReco.browserID, sizeof(retReco.browserID));
                index += sizeof(retReco.browserID); // 申请ID号

                memcpy(sendbuf + index, retReco.deviceID, sizeof(retReco.deviceID));
                index += sizeof(retReco.deviceID); // 设备ID号

                memcpy(sendbuf + index, retReco.compID, sizeof(retReco.compID));
                index += sizeof(retReco.compID); // 组件ID号

                // 遍历扫描内容，按照 文件名称、md5放于发送队列
                printf("the scans file Num is:%d\n", retReco.fileNum);

                for (int i = 0; i < retReco.fileNum; i++)
                {
                    memcpy(sendbuf + index, retReco.scanRets[i].filePath, sizeof(retReco.scanRets[i].filePath));
                    index += sizeof(retReco.scanRets[i].filePath);
                    memcpy(sendbuf + index, retReco.scanRets[i].md5Value, sizeof(retReco.scanRets[i].md5Value));
                    index += sizeof(retReco.scanRets[i].md5Value);
                }
                int ret = tclient.send(sendbuf, len);

                if (ret < 0)
                    RUtil::printError("send scan file info failed!");
                else
                    RUtil::printError("send scan file info success!");

                delete sendbuf;
            }else{
                RUtil::printError("scan file connect error!");
            }
        }
    }

    tclient.closeSocket();
}
