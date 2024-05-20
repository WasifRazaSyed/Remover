#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <baseTsd.h>
#include <WinDef.h>
#include <WinNT.h>
#include <windows.h>
#include <wtsapi32.h>
#include <iostream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

void remove(QString directoryPath);
QString getCurrentUserName();
void checkUpdate();
QString version="v1.1";


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // color is the keyword to pass on the system function, 0 - black background, 7 - White Foreground
    system("color 0A");

    QString user=getCurrentUserName();
    QString path="C:/Windows/Temp";
    remove(path);
    path="C:/Users/"+user+"/AppData/Local/Temp";
    remove(path);
    path="C:/Windows/Prefetch";
    remove(path);
    path="C:/Users/"+user+"/AppData/Local/Google/Chrome/User Data/Default/Cache/Cache_Data";
    remove(path);
    checkUpdate();
    return app.exec();
}

void remove(QString directoryPath)
{
    QDir directory(directoryPath);

    if (directory.exists()) {
        QFileInfoList fileList = directory.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        qDebug()<<"Number of files: "<<fileList.size();

        foreach (const QFileInfo &fileInfo, fileList) {
            if(fileInfo.isFile())
            {
                QFile file(fileInfo.absoluteFilePath());
                if (file.remove()) {
                    qDebug() << "Removed file: " << fileInfo.fileName();
                }
                else {
                    qDebug() << "Failed to remove file: " << fileInfo.fileName();
                }
            }
            else if(fileInfo.isDir())
            {
                QDir insiderDir;
                if(insiderDir.rmdir(fileInfo.absoluteFilePath()))
                {
                    qDebug()<<"Removed folder: "<<fileInfo.fileName();
                }
                else
                {
                    qDebug()<<"Failed to remove folder: "<<fileInfo.fileName();
                }
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

void checkUpdate()
{
    QUrl url_v("https://api.github.com/repos/WasifRazaSyed/Remover/releases/latest");
    QNetworkAccessManager *manager=new QNetworkAccessManager();
    QNetworkRequest request(url_v);
    request.setRawHeader("Authorization", QString("token %1").arg("API_KEY").toUtf8());
    QNetworkReply *reply = manager->get(request);

    QObject::connect(reply, &QIODevice::readyRead, [=]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            reply->abort();
            qDebug()<<"An error occurred while fetching updates.";
            return;
        }

        QByteArray data = reply->readAll();
        QJsonDocument document = QJsonDocument::fromJson(data);

        if (document.isNull()) {
            return;
        }

        QJsonObject rootObject = document.object();
        QString newversion = rootObject.value("tag_name").toString();

        if(version!=newversion && newversion !="")
        {
            std::cout<<"An update is available. Do you want to download it? (y/n): ";
            char input;
            std::cin>>input;
            if(input=='y')
            {

                QUrl downloadPKG("https://github.com/WasifRazaSyed/Remover/raw/main/RemoverSetup.exe?raw=true");
                QNetworkRequest requestPKG(downloadPKG);
                QNetworkReply *replyPKG= manager->get(requestPKG);

                std::cout<<"[ Downloading: 0% ]";
                QObject::connect(replyPKG, &QNetworkReply::downloadProgress, [&](qint64 bytesReceived, qint64 bytesTotal) {
                    int progress_value = static_cast<int>((bytesReceived * 100) / bytesTotal);
                    std::cout<<"\r[ Downloading: "<<progress_value<<"% ]";

                });


                QObject::connect(replyPKG, &QNetworkReply::finished, [=]()
                        {
                    if(replyPKG->error()==QNetworkReply::NoError)
                    {
                        qDebug()<<"Finished";
                        QByteArray PKG=replyPKG->readAll();
                        QString user=getCurrentUserName();
                        QString tempPath = "C:/Users/"+user+"/AppData/Local/Temp/RemoverSetup.exe";
                        QFile file(tempPath);
                        if (file.open(QIODevice::WriteOnly)) {
                            QDataStream out(&file);
                            out.setVersion(QDataStream::Qt_6_1); // Set the stream version to Qt 6.1 to match the x64 architecture
                            out.writeRawData(PKG.constData(), PKG.size()); // Write the binary data to the file
                            file.close();
                            QProcess *installerProcess=new QProcess();
                            qDebug()<<installerProcess->startDetached(tempPath);
                            QCoreApplication::exit(0);
                        }
                    }
                    else
                    {
                        qDebug()<<"Error downloading package.";
                    }
                });
            }
            else if(input=='n')
            {

            }
        }
        else
        {

        }
    });
}
