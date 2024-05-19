#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <baseTsd.h>
#include <WinDef.h>
#include <WinNT.h>
#include <windows.h>
#include <wtsapi32.h>

void remove(QString directoryPath);
QString getCurrentUserName();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString user=getCurrentUserName();
    QString path="C:/Windows/Temp";
    remove(path);
    path="C:/Users/"+user+"/AppData/Local/Temp";
    remove(path);
    path="C:/Windows/Prefetch";
    remove(path);
    path="C:/Users/"+user+"/AppData/Local/Google/Chrome/User Data/Default/Cache/Cache_Data";
    remove(path);
    return a.exec();
}

void remove(QString directoryPath)
{
    QDir directory(directoryPath);
    if (directory.exists()) {
        QFileInfoList fileList = directory.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        qDebug()<<"Number of files: "<<fileList.size();
        foreach (const QFileInfo &fileInfo, fileList) {
            QFile file(fileInfo.absoluteFilePath());
            try
            {
                if (file.remove()) {
                    qDebug() << "Removed file:" << fileInfo.fileName();
                }
                else {
                    qDebug() << "Failed to remove file:" << fileInfo.fileName();
                }
            }
            catch(std::exception &e)
            {
                qDebug()<<e.what();
            }
        }

        qDebug() << "Directory:"<<directoryPath <<"contents cleared successfully.\n";
    } else {
        qDebug() << "Directory:"<<directoryPath <<" does not exist.";
    }
    return;
}

QString getCurrentUserName()
{
    DWORD sessionId = WTSGetActiveConsoleSessionId();
    if (sessionId == 0xFFFFFFFF) {
        // error
        return QString();
    }

    LPWSTR username = nullptr;
    DWORD size = 0;
    if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, sessionId, WTSUserName, &username, &size))
    {
        WCHAR* wUsername = new WCHAR[size / sizeof(WCHAR)];
        wcscpy_s(wUsername, size / sizeof(WCHAR), username);
        QString result = QString::fromWCharArray(wUsername);
        WTSFreeMemory(username);
        delete[] wUsername;
        return result;
    }
    else
    {
        // error
        return QString();
    }
}
