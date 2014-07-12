/*
 * Licence:
 * You can use and modify this file without any restriction.
 * There is no warranty.
 * You also can use the licence from http://www.wtfpl.net/.
 * The original sources can be found on https://github.com/republib.
*/


#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include <QDir>
#include <QDirIterator>
#include <QStringList>
#include <QProcess>
#include <QList>
#include <QThread>

class MainWindow;
class ConverterException{
public:
    ConverterException(const QString& message) : m_message(message)
    {}
    const QString& message() const
    { return m_message; }

private:
    QString m_message;
};

class ConvertLogger{
public:
    virtual bool log(const QString& message) = 0;
    virtual bool error(const QString& message) = 0;
};

class Converter : public QThread
{
    Q_OBJECT
public:
    enum State {
        STATE_UNDEF,
        STATE_STARTING,
        STATE_SUB_TASK_STOPPED,
        STATE_READY,
    };
public:
    Converter(const QString& directory, const QString& targetDirectory,
              int landscapeX, int landscapey,
              int portraitX, int portraitY,
              int squareX, int quality, MainWindow* mainWindows);
    ~Converter();
public:
    bool log(const QString& message);
    bool error(const QString& message);
    void run();
    void stop(){
        m_shouldStop = true;
    }
protected:
    void convertOneFile(const QString& name, qint64 size);
    bool buildArgs();
    void changeState(State state, const QString& info);
private:
    QDir m_dir;
    QDir m_targetDir;
    int m_landscapeWidth;
    int m_landscapeHeight;
    int m_portraitWidth;
    int m_portraitHeight;
    int m_squareWidth;
    int m_quality;
    MainWindow* m_mainWindows;
    QProcess* m_script;
    QStringList m_args;
    bool m_shouldStop;
};


#endif // CONVERTER_HPP
