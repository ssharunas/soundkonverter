
#include "musepackcodecglobal.h"

#include "musepackcodecwidget.h"
#include "musepackconversionoptions.h"
#include "soundkonverter_codec_musepack.h"

#include <QFile>
#include <QStandardPaths>

#define BINARY_ENC "mppenc(v7) or mpcenc(v8)"
#define BINARY_DEC "mppdec(v7) or mpcdec(v8)"

soundkonverter_codec_musepack::soundkonverter_codec_musepack(QObject *parent, const KPluginMetaData &metadata, const QVariantList &args)
    : CodecPlugin(parent)
{
    Q_UNUSED(args)

    binaries[BINARY_ENC] = "";
    binaries[BINARY_DEC] = "";

    allCodecs += "musepack";
    allCodecs += "wav";
}

soundkonverter_codec_musepack::~soundkonverter_codec_musepack()
{
}

QString soundkonverter_codec_musepack::name() const
{
    return global_plugin_name;
}

void soundkonverter_codec_musepack::scanForBackends(const QStringList &directoryList)
{
    binaries[BINARY_ENC] = QStandardPaths::findExecutable("mppenc"); // sv7
    if (binaries[BINARY_ENC].isEmpty())
        binaries[BINARY_ENC] = QStandardPaths::findExecutable("mpcenc"); // sv8

    if (binaries[BINARY_ENC].isEmpty()) {
        for (QList<QString>::const_iterator b = directoryList.begin(); b != directoryList.end(); ++b) {
            if (QFile::exists((*b) + "/mppenc")) {
                binaries[BINARY_ENC] = (*b) + "/mppenc";
                break;
            } else if (QFile::exists((*b) + "/mpcenc")) {
                binaries[BINARY_ENC] = (*b) + "/mpcenc";
                break;
            }
        }
    }

    binaries[BINARY_DEC] = QStandardPaths::findExecutable("mppdec"); // sv7
    if (binaries[BINARY_DEC].isEmpty())
        binaries[BINARY_DEC] = QStandardPaths::findExecutable("mpcdec"); // sv8

    if (binaries[BINARY_DEC].isEmpty()) {
        for (QList<QString>::const_iterator b = directoryList.begin(); b != directoryList.end(); ++b) {
            if (QFile::exists((*b) + "/mppdec")) {
                binaries[BINARY_DEC] = (*b) + "/mppdec";
                break;
            } else if (QFile::exists((*b) + "/mpcdec")) {
                binaries[BINARY_DEC] = (*b) + "/mpcdec";
                break;
            }
        }
    }
}

QList<ConversionPipeTrunk> soundkonverter_codec_musepack::codecTable()
{
    QList<ConversionPipeTrunk> table;
    ConversionPipeTrunk newTrunk;

    newTrunk.codecFrom = "wav";
    newTrunk.codecTo = "musepack";
    newTrunk.rating = 100;
    newTrunk.enabled = (binaries[BINARY_ENC] != "");
    newTrunk.problemInfo = standardMessage("encode_codec,backend", "musepack", "mppenc") + "\n"
        + standardMessage("install_website_backend,url", "mppenc", "http://www.musepack.net");
    newTrunk.data.hasInternalReplayGain = false;
    table.append(newTrunk);

    newTrunk.codecFrom = "musepack";
    newTrunk.codecTo = "wav";
    newTrunk.rating = 100;
    newTrunk.enabled = (binaries[BINARY_DEC] != "");
    newTrunk.problemInfo = standardMessage("decode_codec,backend", "musepack", "mppdec") + "\n"
        + standardMessage("install_website_backend,url", "mppdec", "http://www.musepack.net");
    newTrunk.data.hasInternalReplayGain = false;
    table.append(newTrunk);

    return table;
}

bool soundkonverter_codec_musepack::isConfigSupported(ActionType action, const QString &codecName)
{
    Q_UNUSED(action)
    Q_UNUSED(codecName)

    return false;
}

void soundkonverter_codec_musepack::showConfigDialog(ActionType action, const QString &codecName, QWidget *parent)
{
    Q_UNUSED(action)
    Q_UNUSED(codecName)
    Q_UNUSED(parent)
}

bool soundkonverter_codec_musepack::hasInfo()
{
    return false;
}

void soundkonverter_codec_musepack::showInfo(QWidget *parent)
{
    Q_UNUSED(parent)
}

CodecWidget *soundkonverter_codec_musepack::newCodecWidget()
{
    MusePackCodecWidget *widget = new MusePackCodecWidget();
    return qobject_cast<CodecWidget *>(widget);
}

int soundkonverter_codec_musepack::convert(const QUrl &inputFile,
                                           const QUrl &outputFile,
                                           const QString &inputCodec,
                                           const QString &outputCodec,
                                           const ConversionOptions *_conversionOptions,
                                           TagData *tags,
                                           bool replayGain)
{
    QStringList command = convertCommand(inputFile, outputFile, inputCodec, outputCodec, _conversionOptions, tags, replayGain);
    if (command.isEmpty())
        return BackendPlugin::UnknownError;

    CodecPluginItem *newItem = new CodecPluginItem(this);
    newItem->id = lastId++;
    newItem->process = new KProcess(newItem);
    newItem->process->setOutputChannelMode(KProcess::MergedChannels);
    connect(newItem->process, &KProcess::readyRead, this, &soundkonverter_codec_musepack::processOutput);
    connect(newItem->process, &KProcess::finished, this, &soundkonverter_codec_musepack::processExit);

    newItem->process->clearProgram();
    newItem->process->setShellCommand(command.join(" "));
    newItem->process->start();

    logCommand(newItem->id, command.join(" "));

    backendItems.append(newItem);
    return newItem->id;
}

QStringList soundkonverter_codec_musepack::convertCommand(const QUrl &inputFile,
                                                          const QUrl &outputFile,
                                                          const QString &inputCodec,
                                                          const QString &outputCodec,
                                                          const ConversionOptions *_conversionOptions,
                                                          TagData *tags,
                                                          bool replayGain)
{
    Q_UNUSED(inputCodec)
    Q_UNUSED(tags)
    Q_UNUSED(replayGain)

    if (!_conversionOptions)
        return QStringList();

    QStringList command;
    const ConversionOptions *conversionOptions = _conversionOptions;
    const MusePackConversionOptions *musepackConversionOptions = 0;
    if (conversionOptions->pluginName == name()) {
        musepackConversionOptions = dynamic_cast<const MusePackConversionOptions *>(conversionOptions);
    }

    if (outputCodec == "musepack") {
        command += binaries[BINARY_ENC];
        if (musepackConversionOptions && musepackConversionOptions->data.preset != MusePackConversionOptions::Data::UserDefined) {
            if (musepackConversionOptions->data.preset == MusePackConversionOptions::Data::Telephone) {
                command += "--telephone";
            } else if (musepackConversionOptions->data.preset == MusePackConversionOptions::Data::Thumb) {
                command += "--thumb";
            } else if (musepackConversionOptions->data.preset == MusePackConversionOptions::Data::Radio) {
                command += "--radio";
            } else if (musepackConversionOptions->data.preset == MusePackConversionOptions::Data::Standard) {
                command += "--standard";
            } else if (musepackConversionOptions->data.preset == MusePackConversionOptions::Data::Extreme) {
                command += "--extreme";
            } else if (musepackConversionOptions->data.preset == MusePackConversionOptions::Data::Insane) {
                command += "--insane";
            } else if (musepackConversionOptions->data.preset == MusePackConversionOptions::Data::Braindead) {
                command += "--braindead";
            }
        } else {
            command += "--quality";
            command += QString::number(conversionOptions->quality);
        }
        if (conversionOptions->pluginName == name()) {
            command += conversionOptions->cmdArguments;
        }
        command += "\"" + escapeUrl(inputFile) + "\"";
        command += "\"" + escapeUrl(outputFile) + "\"";
    } else {
        command += binaries[BINARY_DEC];
        command += "\"" + escapeUrl(inputFile) + "\"";
        command += "\"" + escapeUrl(outputFile) + "\"";
    }

    return command;
}

float soundkonverter_codec_musepack::parseOutput(const QString &output)
{
    // sv7
    //  47.4  143.7 kbps 23.92x     1:43.3    3:38.1     0:04.3    0:09.1     0:04.8

    // sv8
    // 37.5
    // 171.2 kbps
    // 23.03x
    // 0:36.1
    // 1:36.5
    // 0:01.5
    // 0:04.1
    // 0:02.6

    static QRegularExpression reg("(\\d+\\.\\d)\\s+\\d+\\.\\d kbps");
    if (output.contains(reg)) {
        return reg.match(output).captured(1).toFloat();
    }

    return -1;
}

ConversionOptions *soundkonverter_codec_musepack::conversionOptionsFromXml(QDomElement conversionOptions, QList<QDomElement> *filterOptionsElements)
{
    MusePackConversionOptions *options = new MusePackConversionOptions();
    options->fromXml(conversionOptions, filterOptionsElements);
    return options;
}

K_PLUGIN_FACTORY_WITH_JSON(soundkonverter_codec_musepackFactory, "soundkonverter_codec_musepack.json", registerPlugin<soundkonverter_codec_musepack>();)

#include "soundkonverter_codec_musepack.moc"
