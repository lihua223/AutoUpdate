#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtXml>
#include <QDomDocument>
#include <QFile>
#include <QDomNodeList>
#include <QDebug>
#include <QDomNode>
#include <QMessageBox>
#include <QSettings>
#include <QDir>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

#define localXml  0
#define onlineXml 1
#define LOCALPath "FTP://liuy:liuy@192.168.1.213:21/abc/version.xml"

// 定义xml结构体，获取必要的tag
typedef struct xml
{
    QString name;
    QString num;
    QString downpath;
}XML;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

    // 解析xml文件
    void parsexml(const QString &path, QVector<XML> &vector, int flag = localXml);

    // xml节点转XML对象
    XML &loadxml(QDomNode node, XML &xml);

    // 解析子节点
    void parsechildNode(QDomNode &node, QVector<XML> &vector, int flag = localXml);

    // 获取下载文件列表
    void analyzeDownloadList();

    // 比较版本
    bool comVersion(QString version1, QString version2);

    // 创建文件夹
    void createDirectory(QString tempFile);

    void start_up();

public:
    // 下载并保存文件到临时文件夹
    void getSomething(QUrl url);

    // 请求下载
    void startRequest(QUrl url);

    // 下载完成后动作
    void DownLoadComplete();

    // 删除文件夹
    bool removeTempFile(const QString &tempFile);

    // 将临时文件夹中文件拷贝到目标文件夹
    bool copyFileToPath(const QString &tempFile, const QString &tagFile
                                    , bool coverFileIfExist);
    // 备份本地文件
    void backupFile(QString fileName);

    // 恢复本地文件
    void restoreBackupFile();

    // 开始更新
    bool beginUpdate();

    void setUser_ini();

private slots:
    // 下载完成
    void downFinished();

    // 准备下载
    void downReadyRead();

private:
    Ui::Dialog *ui;

    QVector<XML> vectorlocal;
    QVector<XML> vectordownload;
    QList<QUrl> qlist;
    QList<QString> qlFileName;
    QList<QUrl>::Iterator it;

    QUrl m_url;             // URL地址
    QFile *downloadFile;    // 文件下载地址

    QNetworkAccessManager *downloadManager;
    QNetworkReply *downloadReply;

    QString m_tempPath;
    QString m_backupPath;
    QString version[2];
};

#endif // DIALOG_H
