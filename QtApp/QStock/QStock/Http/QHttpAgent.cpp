#include "QHttpAgent.h"

void QHttpAgent::setIdb(StockIdDB *value)
{
    idb = value;
    updateUri();
}

void QHttpAgent::updateUri()
{
    if(idb){
        uri.clear();
        uri=QString("/list=");
        if(idb->isEmpty()){
            /* 默认添加上证和深圳指数 */
            idb->append("sz399001");
            idb->append("sh000001");
        }

        for(int cnt = 0; cnt < idb->count(); cnt++){
            uri = uri+idb->at(cnt)+",";
        }
        /* remove last ',' */
        uri.remove(uri.length()-1,1);
    }
}

QHttpAgent::QHttpAgent(QString m_host)
{
    host = m_host;
    sep = '\n';
    eachFetchCnt = 5;

    http = new QHttp(this);
    /*"http://hq.sinajs.cn"*/
    http->setHost(host);
    url=QString(m_host);

    connect(http, SIGNAL(done(bool)),this, SLOT(slot_httpDone(bool)));
    connect(http, SIGNAL(dataReadProgress(int,int)),this, SLOT(on_dataReadProgress(int,int)));
    connect(http, SIGNAL(readyRead(QHttpResponseHeader)),this, SLOT(on_readyRead(QHttpResponseHeader)));
}

void QHttpAgent::slot_httpDone(bool done)
{
    emit httpDone(done);
}

void QHttpAgent::on_dataReadProgress(int done, int total)
{
    emit dataReadProgress(done,total);
}

void QHttpAgent::on_readyRead(QHttpResponseHeader )
{
    int lastSep = 0;
    buffer.append(http->readAll());
    lastSep = buffer.lastIndexOf(sep);
    if(lastSep > 0){
        emit readyRead(buffer.left(lastSep+1));
        buffer.remove(0,lastSep+1);
    }
}

STATUS QHttpAgent::fetchStockData()
{
    if(!idb){
        return STATUS_NULL;
    }
    if(idb->isEmpty()){
        return STATUS_NULL;
    }
    if(this->isRunning()){
        return STATUS_FAILED;
    }
    this->start();
    return STATUS_OK;
}

void QHttpAgent::run()
{
    int left = idb->size();
    int size = idb->size();
    int count = 0;
    QString uri;

    while(left >= eachFetchCnt){
        QHttpRequestHeader header;
        uri = QString("/list=");

        count = eachFetchCnt;
        while(count--){
            uri.append(idb->at(size-left));
            uri.append(",");
            left--;
        }
        uri.remove(uri.length()-1,1);

        header.setRequest("GET",uri);
        header.setValue("Host", host);
        http->setHost(host);
        http->request(header);
    }

    if(left > 0){
        QHttpRequestHeader header;
        uri = QString("/list=");

        count = left;
        while(count--){
            uri.append(idb->at(size-left));
            uri.append(",");
            left--;
        }
        uri.remove(uri.length()-1,1);

        header.setRequest("GET",uri);
        header.setValue("Host", host);
        http->setHost(host);
        http->request(header);
    }
}
