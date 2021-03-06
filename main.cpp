#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QDataStream>
#include <QTextStream>
#include <QByteArray>
#include <QDebug>

#define echo QTextStream(stderr) <<

using namespace std;

/**
 * @brief crc16Add
 * @param data
 */
uint16_t crc_calc(char *buff, unsigned int len)
{
    uint16_t crc = 0xFFFF;
    uint16_t crcshr8;
    uint16_t crcshl8;
    uint16_t c1shl12;
    uint16_t c1shl05;
    uint16_t c1shl00;
    uint8_t  c1;
    uint8_t  byte;

    while(len--)
    {
       byte   = (uint8_t) *buff++;

       crcshr8 = crc >> 8;
       crcshl8 = crc << 8;

       c1 = crcshr8 ^ byte;
       c1 = c1 ^ (c1 >> 4);

       c1shl05 = (uint16_t)(c1 << 5);
       c1shl12 = (uint16_t)(c1 << 12);
       c1shl00 = (uint16_t)(c1 << 0 );

       crc = crcshl8 ^ c1shl12  ^ c1shl05 ^ c1shl00;
    }

    return crc;
}

int main(int argc, char *argv[])
{

    echo "\nSC-Develop bin-crc-cli utility v1.0\n";
    echo "Copyright (c) 2021 (MIT) Ing. Salvatore Cerami - dev.salvatore.cerami@gmail.com\n";
    echo "https://github.com/sc-develop - git.sc.develop@gmail.com\n";

    if (argc < 3)
    {
        qDebug() << "\n" << "Usage: bin-crc-cli <filename> <size>" << "\n";

        exit(0);
    }

    QFile file(argv[1]);

    int    crcsize   = 2;                              // 2 bytes crc size
    qint64 destsize  = QString(argv[2]).toInt();       // filled file size (crc 2 bytes included)
    qint64 filesize  = file.size();                    // file size
    qint64 fillbytes = destsize - filesize - crcsize;  // byte to fill

    if (fillbytes<0)
    {
        qDebug()  << "\n\n" << "Attenzione: la dimensione del file destinazione non può essere inferione a " <<  QString::number(filesize + crcsize) << " Bytes" << "\n\n";

        exit(0);
    }

    QString path = QFileInfo(file.fileName()).absolutePath() + "/";

    QString destfilename = path + QFileInfo(file.fileName()).completeBaseName() + "-crc.bin";

    if (QFile::exists(destfilename))
    {
        if (!file.remove(destfilename))
        {
            qDebug() << "\n\n" << "Errore: Impossible sovrascrivere il file " + destfilename << "\n\n";
            exit(0);
        }
    }

    if (!file.copy(destfilename)) // esegue la copia il file sorgente nel file destinazione (in questo momento sono uguali)
    {
        qDebug() << "\n\n" << "Errore: Impossible copiare il file " + file.fileName() + " in " + destfilename << "\n\n";
        exit(0);
    }

    file.setFileName(destfilename);

    if (file.open(QIODevice::Append))
    {
        QDataStream out(&file);

        while (fillbytes--)
        {
           out << (uint8_t)0xFF;
        }

        file.close();

        QString hexcrc;

        if (file.open(QFile::ReadOnly))
        {
           QByteArray buff = file.readAll();

           // calc file crc *******************************************************************************

           int blen = buff.length();

           /*crc16Init();

           for (int n=0; n<blen; n++)
           {
              uint8_t c = (uint8_t) buff.at(n);
              crc16Add(c);
           }

           uint16_t crc = crcGet();*/

           uint16_t crc = crc_calc(buff.data(),blen);

           hexcrc = QString("%1").arg(crc,4,16,QChar('0')).toLocal8Bit().toUpper();

           file.close();

           // write crc to end of file *********************************************************************

           if (file.open(QIODevice::Append))
           {
              char crch = (char) (crc >> 8);
              char crcl = (char) (crc >> 0);

              file.write(&crch,1);
              file.write(&crcl,1);

              file.close();
           }
        }

        qDebug() << "\n\n" << "Completato => File generato: " << QFileInfo(destfilename).fileName() << " - " << file.size() << " Bytes - CRC: " << hexcrc << "\n\n";;
   }
   else
   {
     qDebug() << "\n\n" << "Error: Impossibile aprire/creare il file: " + file.fileName() << "\n\n";
   }

    exit(0);
}
