/*
 * Licence:
 * You can use and modify this file without any restriction.
 * There is no warranty.
 * You also can use the licence from http://www.wtfpl.net/.
 * The original sources can be found on https://github.com/republib.
*/


#include "converter.hpp"
#include "mainwindow.hpp"

/** @mainpage A converter for images.
 *
 * @section intro_sec Introduction
 *
 * The user can determine the maximum width/length for the three formats:
 * <ul>
 * <li>landscape: width greater than height</li>
 * <li>portrait: witdh lower then height</li>
 * <li>square: width equal to height</li>
 * </ul>
 * At a given directory all images will be converted by this rules.
 * The created files will be stored in another directory (normally a
 * subdirectory of the source directory).
 *
 * @subsection conv_sec Conversion Details
 *
 * If an image is lower than the given limits it will be copied without
 * conversion
 * This program is only the GUI of an Perl script which does the conversion.
 * The script uses the
 *
 * @section install_sec Installation
 *
 * Copy rplshrink and img_conv.pl into a directory inserted in the executable
 * path, e.g. /usr/local/bin and give them the right "executable".
 *
 * @subsection req_sec Requirements
 *
 * <ul>
 * <li>ImageMagick</li>
 * <li>Perl</li>
 * </ul>
 *
 * @section prog_sec Programming Features
 *
 * This is a example for a complete QT application with the following features:
 * <ul>
 * <li>C++</li>
 * <li>QT</li>
 * <li>Threads</li>
 * <li>Using GUI wizzard</li>
 * <li>Abstract class as interface</li>
 * <li>Calling an external script</li>
 * <li>Lists the output of the external list line by line at the moment
 *  the line is created</li>
 * </ul>
 *
 * @section version_sec Release Notes
 *
 * <ul>
 * <li>V1.0 Basic version. Predefined (static) values of the combo boxes.
 *      User language is German only.
 * </li>
 * </ul>
 *
 * @section Licence
 *
 * You can use and modify this file without any restriction.
 * There is no warranty.
 * You also can use the licence from http://www.wtfpl.net/.
 * The original sources can be found on https://github.com/republib
 */

/** @class ConverterException converter.hpp "converter.hpp"
 *
 * @brief Implements a specific exception for the converter.
 *
 * Will be used on errors.
*/

/** @class ConvertLogger converter.hpp "converter.hpp"
 *
 * @brief Implements an abstract base class for logging.
*/


/** @class Converter converter.hpp "converter.hpp"
 *
 * @brief Implements a converter for images.
 *
 * Search all images in a given path and converts them into the given format.
 *
 * The task is done in a thread because it can take long time.
 *
 * The thread can be stopped from outside, then the conversion of the current
 * image will be finished and no more files will be processed.
 *
 */

/**
 * @brief Constructor.
 *
 * @param directory         the source directory. The images will be searched here
 * @param targetDirectory   the target directory
 * @param landscapeX        0 or the given maximal width for landscape format
 * @param landscapeY        0 or the given maximal height for landscape format
 * @param portraitX         0 or the given maximal width for portrait format
 * @param portraitY         0 or the given maximal height for landscape format
 * @param squareX           0 or the given maximal width for square format
 * @param quality           the quality factor: 1..100. Higher value is better
 *                          but larger
 * @param mainWindow        the window. Will be used for output
 */
Converter::Converter(const QString& directory, const QString& targetDirectory,
                     int landscapeX, int landscapeY,
                     int portraitX, int portraitY,
                     int squareX, int quality,
                     MainWindow* mainWindow) :
    m_dir(directory),
    m_targetDir(targetDirectory.indexOf(QDir::separator()) >= 0
        ? targetDirectory
        : directory + QDir::separator() + targetDirectory),
    m_landscapeWidth(landscapeX),
    m_landscapeHeight(landscapeY),
    m_portraitWidth(portraitX),
    m_portraitHeight(portraitY),
    m_squareWidth(squareX),
    m_quality(quality),
    m_mainWindows(mainWindow),
    m_script(new QProcess()),
    m_shouldStop(false)
{
}

/**
 * @brief Destructor.
 */
Converter::~Converter()
{
    delete m_script;
}

/**
 * @brief Appends an argument to the argument list.
 *
 * @param args      IN/OUT: the argument list
 * @param prefix    the string starting the argument
 * @param value     the value of the argument
 */
void addArg(QStringList& args, const char* prefix, int value)
{
    QString arg;
    arg.sprintf("%s%d", prefix, value);
    args << arg;
}

/**
 * @brief Builds the arguments of the script call.
 *
 * @return  true: OK<br>
 *          false: error occurred
 */
bool Converter::buildArgs()
{
    bool rc = true;
    m_args.clear();
    m_args << "-v";
    if (m_landscapeHeight > 0)
        addArg(m_args, "-lw", m_landscapeHeight);
    else if (m_landscapeWidth > 0)
        addArg(m_args, "-lh", m_landscapeWidth);
    else
        rc = error("keine/falsche Maximalangabe beim Breitformat");

    if (m_portraitWidth > 0)
        addArg(m_args, "-pw", m_portraitWidth);
    else if (m_portraitHeight > 0)
        addArg(m_args, "-ph", m_portraitHeight);
    else
        rc = error("keine/falsche Maximalangabe beim Hochformat");

    if (m_squareWidth > 0)
        addArg(m_args, "-sw", m_squareWidth);
    else
        rc = error("keine/falsche Maximalangabe beim Hochformat");

    if (m_quality < 10 || m_quality > 100)
        rc = error("keine/falsche Qualität");
    else
        addArg(m_args, "-q", m_quality);

    return rc;
}

/**
 * @brief Changes the state of the thread.
 *
 * @param state     the new state
 * @param info      an info about the change
 */
void Converter::changeState(Converter::State state, const QString& info)
{
    m_mainWindows->on_threadStateChanged(state, info);
}

/**
 * @brief Runs the thread's task.
 *
 * <ul>
 *<li>Makes the target directory (if necessary)</li>
 *<li>Search the images *.png / *.jpg and converts them</li>
 *<ul>
 */
void Converter::run()
{
    QString msg;
    int no = 0;
    try {
        if (! m_dir.exists())
            error("Verzeichnis existiert nicht: " + m_dir.absolutePath());
        if (! m_targetDir.exists()){
            QString parentName = m_targetDir.path();
            QString subdir = m_targetDir.dirName();
            QDir parent(m_targetDir.path());
            parent.cdUp();
            parent.mkdir(subdir);
        }
        if (! m_targetDir.exists()){
            error("Kann Zielverzeichnis nicht anlegen: " + m_targetDir.absolutePath());
        }
        QStringList names;
        names.append("*.jpg");
        names.append("*.JPG");
        names.append("*.png");
        names.append("*.PNG");
        changeState(Converter::STATE_STARTING, "");
        buildArgs();
        m_shouldStop = false;
        QDirIterator it(m_dir.absolutePath(), names);
        while(it.hasNext()){
            if (m_shouldStop){
                log("Abbruch durch Benutzer");
                break;
            }
            no++;
            it.next();
            QString path = it.path();
            qint64 length = it.fileInfo().size();
            path += QDir::separator();
            path += it.fileName();
            convertOneFile(path, length);

            msg.sprintf("%4d Datei(en) konvertiert", no);
            changeState(Converter::STATE_SUB_TASK_STOPPED, msg);
        }
    } catch(ConverterException exc){
        log("Abarbeitung wegen Fehlers gestoppt: " + exc.message());
    }
    msg.sprintf("%d Datei(en) konvertiert", no);
    changeState(Converter::STATE_READY, msg);
}

/**
 * @brief Search for the Perl script which make the conversion itself.
 *
 * @param node  the name of script without path.
 * @return      the full name of the script (with path)
 */
QString findScript(const QString& node)
{
    static QString rc;
    if (rc.isEmpty()){

        QDir dir = QDir::current();
        QFile scriptFile(dir.filePath(node));
        if (! scriptFile.exists())
        {
            extern char** g_argv;
            dir.setPath(g_argv[0]);
            dir.cdUp();
            scriptFile.setFileName(dir.filePath(node));
        }
        if (scriptFile.exists())
            rc = scriptFile.fileName();
    }
    return rc;
}
/**
 * @brief Converts the size into a human readable string.
 *
 * @param size  the size in bytes
 * @return      the size as human readable string, e.g. "2MiByte"
 */
QString sizeToString(qint64 size)
{
    QString rc;
    if (size < 10*1024)
        rc.sprintf("%d Bytes", (int) size);
    else if (size < qint64(10*1024*1024))
        rc.sprintf("%d KiBytes", (int) (size / 1024));
    else if (size < qint64(10*1024*1024)*1024)
        rc.sprintf("%d MiBytes", (int) (size / 1024 / 1024));
    else
        rc.sprintf("%d GiBytes", (int) (size / 1024 / 1024 / 1024));
    return rc;
}

/**
 * @brief Converts one file.
 *
 * @param file  the file's name
 * @param size  the size of the file (in byte)
 */
void Converter::convertOneFile(const QString& file, qint64 size)
{
    QString node("img_trans.pl");
    log(file + " " + sizeToString(size));
    QString script(findScript(node));
    if (script.isEmpty())
        error("Script nicht gefunden: " + node);
    else {
        QStringList args;
        args << script << m_args << file << m_targetDir.absolutePath();
        m_script->start("/usr/bin/perl", args);
        QByteArray output;
        while (m_script->waitForReadyRead()){
            output = m_script->readAll();
            QList<QByteArray> lines = output.split('\n');
            QList<QByteArray>::iterator it;
            for (it = lines.begin(); it != lines.end(); it++){
                log(*it);
            }
        }
        output = m_script->readAllStandardError();
        if (!output.isEmpty())
            error(output);
        m_script->close();
    }
}

/**
 * @brief Logs a message.
 *
 * @param message   the message to log
 * @return          <code>true</code>
 */
bool Converter::log(const QString& message)
{
    printf ("%s\n", message.toUtf8().constData());
    m_mainWindows->log(message);
    return true;
}

/**
 * @brief Logs an error message.
 *
 * @param message   the message to log
 * @return          <code>false</code>
 * @throws ConverterException
 */
bool Converter::error(const QString& message)
{
    m_mainWindows->log(message);
    throw ConverterException(message);
    return false;
}

