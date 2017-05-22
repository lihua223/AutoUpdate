#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    downloadFile = NULL;
    m_tempPath = "./temp/";
    m_backupPath = "./backup/";
    downloadReply = NULL;
    downloadManager = new QNetworkAccessManager(this);

    qlFileName.clear();
    qlist.clear();
    vectordownload.clear();
    vectorlocal.clear();

    start_up();
}

Dialog::~Dialog()
{
    delete ui;


    if (downloadFile)
    {
        downloadFile->close();
        delete downloadFile;
        downloadFile = NULL;
    }

    if (downloadReply)
    {
        downloadReply->deleteLater();
        downloadReply = NULL;
    }

    if (downloadManager)
    {
        downloadManager->deleteLater();
        downloadManager = NULL;
    }
}

void Dialog::start_up()
{
    createDirectory(m_tempPath);
    createDirectory(m_backupPath);
    QUrl url(QUrl(LOCALPath));
    //m_down_request = true;
    getSomething(url);
}

void Dialog::parsexml(const QString &path, QVector<XML> &vector, int flag)
{
    QDomDocument doc;
    QFile pathfile(path);

    if (!pathfile.exists())
    {
        //qDebug() << "file not find";
        qApp->quit();
        return;
    }
    pathfile.open(QIODevice::ReadOnly);

    QString errorStr;
    int errorLine;
    int errorCol;
    if (!doc.setContent(&pathfile, false, &errorStr, &errorLine, &errorCol))
    {
        qApp->quit();
        qDebug() << "DOC" << ":::" << errorStr << errorLine << errorCol;
        pathfile.close();
    }
    pathfile.close();

    QDomElement root = doc.documentElement();
    //qDebug() << "root" << root.nodeName();
    QDomNode totalNode = root.firstChild();
    //qDebug() << "first" << firstNode.nodeName();

    while (!totalNode.isNull())
    {
        QDomElement element = totalNode.toElement();
        //qDebug() << totalNode.nodeName();
        //qDebug() << "-------------" << element.text();

        if (!element.isNull())
        {
            parsechildNode(totalNode, vector, flag);
        }

        totalNode = totalNode.nextSibling();
    }
}

void Dialog::parsechildNode(QDomNode &node, QVector<XML> &vector, int flag)
{
    QDomNodeList totallist = node.childNodes();

    for (int i = 0; i < totallist.count(); i++)
    {
        //qDebug() << totallist.at(i).nodeName();
        if (totallist.at(i).childNodes().count() > 0)
        {
            QDomNodeList childlist = totallist.at(i).childNodes();
            XML xml;
            for (int j = 0; j < childlist.count(); j++)
            {
//                qDebug() << j << "-----------------" << childlist.at(j).nodeName();
//                qDebug() << j << "   " << childlist.at(j).toElement().text();

                if (childlist.at(j).toElement().text() == "update.exe")
                {
                    if (flag == 0)
                    {
                        version[0] = childlist.at(j).nextSibling().toElement().text();
                        //qDebug() << version[0];
                    }else
                    {
                        version[1] = childlist.at(j).nextSibling().toElement().text();
                        //qDebug() << version[1];
                    }
                    break;
                }

                loadxml(childlist.at(j), xml);
            }
            vector.push_back(xml);
        }
    }
}

XML &Dialog::loadxml(QDomNode node, XML &xml)
{
    if (node.toElement().tagName() == "name")
    {
        xml.name = node.toElement().text();
    }
    if (node.toElement().tagName() == "num")
    {
        xml.num = node.toElement().text();
    }
    if (node.toElement().tagName() == "downpath")
    {
        xml.downpath = node.toElement().text();
    }

    //qDebug() << node.toElement().text();

    return xml;
}

void Dialog::analyzeDownloadList()
{
    parsexml("./version.xml", vectorlocal, 0);
    parsexml(m_tempPath+"version.xml", vectordownload, 1);

    if (comVersion(version[0], version[1]))
    {
        qDebug() << "same version";
        qApp->quit();
        return;
    }

    int length1 = vectorlocal.count();
    int length2 = vectordownload.count();

    qDebug() << length1;

    for(int i = 1; i < length1; i++)
    {
        if ((vectorlocal.at(i).num.toInt()) < (vectordownload.at(i).num.toInt()))
        {
            qlist.append(QUrl(vectordownload.at(i).downpath));
            //qDebug() << QUrl(vectordownload.at(i).downpath);
        }
    }

    if (vectorlocal.count() < vectordownload.count())
    {
        for (int i = length1; i < length2; i++)
        {
            qlist.append(QUrl(vectordownload.at(i).downpath));
        }
    }
}

bool Dialog::comVersion(QString version1, QString version2)
{
    QStringList listLocal = version1.split(".");
    QStringList listDownload = version2.split(".");
    //qDebug() << xmlDownload.Versionnum;
    //createSharedMemory();
    //WriteSharedMemory();

    for (int i = 0; i < listLocal.count(); i++)
    {
        if (((QString)listLocal.at(i)).toInt() == ((QString)listDownload.at(i)).toInt())
        {
            //qDebug() << "comversion" << ((QString)listLocal.at(i)).toInt();
            if (i == listLocal.count() - 1)
            {
                return true;
            }
            continue;
        }else if (((QString)listLocal.at(i)).toInt() > ((QString)listDownload.at(i)).toInt())
        {
            //qDebug() << "onlineXml version lower";
            // 线上版本号比本地版本号低，不需要更新
            return true;
        }else
        {
            //qDebug() << "onlineXml version higher";
            return false;
        }
    }

    return true;
}

void Dialog::createDirectory(QString tempFile)
{
    QDir temp;
    bool exist = temp.exists(tempFile);

    if (exist)
    {
        qDebug() << "file exist";
    }else
    {
        temp.mkdir(tempFile);
    }
}

void Dialog::getSomething(QUrl url)
{
    if (url.isEmpty())
    {
        qDebug() << url;
        qApp->quit();
        return;
    }

    if (downloadFile)
    {
        return;
    }

    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();
    qlFileName.append(fileName);
    downloadFile = new QFile(m_tempPath + fileName);
    if (!downloadFile->open(QIODevice::WriteOnly | QIODevice::ReadOnly))
    {
        qDebug() << "getsomething" << "downloadfile error";
        qApp->quit();
        //m_downPath = "";
        return;
    }

    // 请求下载
    startRequest(url);
}

void Dialog::startRequest(QUrl url)
{
    qDebug()<<"Url:::::::"<<url;
    QNetworkRequest quest(url);
    //quest->setUrl(url);
    downloadReply = downloadManager->get(quest);

    if (downloadReply)
    {
        qDebug() << "----------------------start request";
        connect(downloadReply, SIGNAL(readyRead()), this, SLOT(downReadyRead()));

//        connect(downloadReply, SIGNAL(error(QNetworkReply::NetworkError)), this
//                , SLOT(downloadError(QNetworkReply::NetworkError)));

        // disconnect(downloadReply, SIGNAL(finished()), this, SLOT(downFinished()));
        connect(downloadReply, SIGNAL(finished()), this, SLOT(downFinished()));
    }
}

void Dialog::DownLoadComplete()
{
    qDebug() << "load qlist";
    if (qlist.size() == 0)
    {
        analyzeDownloadList();
        if (qlist.size() == 0)
        {
            qDebug() << "qlist empty";
            qApp->quit();
            return;
        }

        it = qlist.begin();
        m_url = *it;
        getSomething(m_url);
        it++;
    }else if (it != qlist.end())
    {
        m_url = *it;
        getSomething(m_url);
        it++;
    }else
    {
        qDebug() << tr("更新完成");

        QMessageBox box(QMessageBox::Question, "提示",
                        "发现新版本，点击确定升级到新版本，点击取消保持现有版本");
        box.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
        box.setButtonText(QMessageBox::Ok, QString("确 定"));
        box.setButtonText(QMessageBox::Cancel, QString("取 消"));
        box.setWindowFlags(Qt::WindowStaysOnTopHint);
        box.activateWindow();
        int ret = box.exec();
        if(ret == QMessageBox::Cancel)
        {
            removeTempFile(m_tempPath);
            qApp->quit();
            return;
        }

        QProcess process;
        process.execute("TASKKILL /IM MicroCourse.exe /F");
        process.waitForFinished();
        process.close();

        bool bSuccess = beginUpdate();
        if (!bSuccess)
        {
            qDebug() << "updata failure";
            restoreBackupFile();
        }else
        {
            qDebug() << "copy succuss";
            setUser_ini();
            QProcess *myprocess = new QProcess();
            myprocess->start("./MicroCourse.exe", NULL);
            myprocess->waitForFinished();
            qApp->quit();
        }
    }
}

bool Dialog::removeTempFile(const QString &tempFile)
{
    if (tempFile.isEmpty())
        {
            return false;
        }

        QDir dir(tempFile);
        if (!dir.exists())
        {
            return true;
        }

        dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        QFileInfoList filelist = dir.entryInfoList();

        foreach (QFileInfo fi, filelist) {
            if (fi.isFile())
            {
                fi.dir().remove(fi.fileName());
                //qDebug() << fi.fileName();
            }else
            {
                // 文件夹多层嵌套
                removeTempFile(fi.absoluteFilePath());
            }
        }

        //qDebug() << dir.absolutePath();
        return dir.rmpath(dir.absolutePath());
}

bool Dialog::copyFileToPath(const QString &tempFile, const QString &tagFile, bool coverFileIfExist)
{
    QDir tempDir(tempFile);
        QDir targetDir(tagFile);

        if(!targetDir.exists())
        {
            if(!targetDir.mkdir(targetDir.absolutePath()))
            {
                return false;
            }
        }

        QFileInfoList fileInfoList = tempDir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        //qDebug() << fileInfoList.count();

        for (int i = 0; i < fileInfoList.count(); i++)
        {
            //qDebug() << fileInfoList.at(i).fileName();
        }

        foreach(QFileInfo fileInfo, fileInfoList)
        {
            //qDebug() << "----copy" << fileInfo.fileName();
            if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                continue;

            //qDebug() << fileInfo.fileName();
            if(fileInfo.isDir())
            {
                if(!copyFileToPath(fileInfo.filePath(),
                    targetDir.filePath(fileInfo.fileName()),
                    coverFileIfExist))
                {
                    return false;
                }
            }else
            {
                //qDebug() << fileInfo.fileName();
                if(coverFileIfExist && targetDir.exists(fileInfo.fileName()))
                {
                    targetDir.remove(fileInfo.fileName());
                }

                // 进行文件copy
                if(!QFile::copy(fileInfo.filePath(),
                    targetDir.filePath(fileInfo.fileName())))
                {
                        return false;
                }
            }
        }

        return true;
}

void Dialog::backupFile(QString fileName)
{
    QString backupPath = m_backupPath + fileName;
    QFileInfo fileinfo(fileName);
    //qDebug() << fileinfo.absolutePath();
    QFile::copy(fileName, backupPath);
}

void Dialog::restoreBackupFile()
{
    copyFileToPath(m_backupPath, QDir::currentPath(), true);
    removeTempFile(m_tempPath);
    removeTempFile(m_backupPath);
}

bool Dialog::beginUpdate()
{
    foreach (QString filename, qlFileName)
    {
        //qDebug() << QDir::currentPath();
        backupFile(filename);
        QDir targetDir("./");

        if (filename.contains("version.xml"))
        {
            targetDir.remove(filename);
        }
    }

    bool b = copyFileToPath(m_tempPath, QDir::currentPath(), true);
    if (b)
    {
        removeTempFile(m_tempPath);
        removeTempFile(m_backupPath);
    }

    return b;
}

void Dialog::setUser_ini()
{
    QSettings setting("./user.ini", QSettings::IniFormat);
    QMessageBox::information(this, "confirm", setting.value("config/skin").toString());
    QMessageBox::information(this, "name", setting.value("/version/num").toString());
    setting.setValue("/version/num", version[1]);
}

void Dialog::downReadyRead()
{
    if (downloadFile)
    {
        downloadFile->write(downloadReply->readAll());
    }
}

void Dialog::downFinished()
{
    downloadFile->flush();
    downloadFile->close();

    QVariant redirectionTarget = downloadReply->attribute(QNetworkRequest
                                                          ::AuthenticationReuseAttribute);
    qDebug() << "---------------";
//    qDebug() << redirectionTarget;
    if (downloadReply->error())
    {
        qDebug() << "--------" << downloadReply->error();
    }

    if (!redirectionTarget.isNull())
    {
        qDebug() << "restart";
        QUrl newurl = m_url.resolved(redirectionTarget.toUrl());
        m_url = newurl;
        downloadReply->deleteLater();
        downloadFile->open(QIODevice::WriteOnly);
        startRequest(m_url);

        //return;
    }else
    {
        qDebug() << "--------------------download finish";
        delete downloadFile;
        downloadFile = NULL;
        downloadReply->deleteLater();
        downloadReply = NULL;

        DownLoadComplete();
    }
}


