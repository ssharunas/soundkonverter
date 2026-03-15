#include "lamecodecglobal.h"

#include "lamecodecwidget.h"
#include "lameconversionoptions.h"
#include "soundkonverter_codec_lame.h"

#include <KComboBox>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPageDialog>
#include <KSharedConfig>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QWidget>

class ConfigDialog : public KPageDialog
{
public:
    explicit ConfigDialog(soundkonverter_codec_lame *plugin, QWidget *parent)
        : KPageDialog(parent)
        , plugin(plugin)
    {
        setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
        setWindowTitle(i18n("Configure %1", plugin->name()));

        QWidget *configDialogWidget = new QWidget(this);
        QHBoxLayout *configDialogBox = new QHBoxLayout(configDialogWidget);
        configDialogBox->addWidget(new QLabel(i18n("Stereo mode:")));

        configDialogSamplingRateQualityComboBox = new KComboBox(configDialogWidget);
        configDialogSamplingRateQualityComboBox->addItem(i18n("Automatic"), "automatic");
        configDialogSamplingRateQualityComboBox->addItem(i18n("Joint Stereo"), "joint stereo");
        configDialogSamplingRateQualityComboBox->addItem(i18n("Simple Stereo"), "simple stereo");
        configDialogSamplingRateQualityComboBox->addItem(i18n("Forced Joint Stereo"), "forced joint stereo");
        configDialogSamplingRateQualityComboBox->addItem(i18n("Dual Mono"), "dual mono");
        configDialogBox->addWidget(configDialogSamplingRateQualityComboBox);

        connect(this, &ConfigDialog::accepted, this, &ConfigDialog::save);
        connect(buttonBox()->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &ConfigDialog::resetDefault);

        this->addPage(configDialogWidget, "");
    }

    void setStereoMode(QString stereoMode)
    {
        configDialogSamplingRateQualityComboBox->setCurrentIndex(configDialogSamplingRateQualityComboBox->findData(stereoMode));
    }

private:
    KComboBox *configDialogSamplingRateQualityComboBox;
    soundkonverter_codec_lame *plugin;

    void resetDefault()
    {
        configDialogSamplingRateQualityComboBox->setCurrentIndex(configDialogSamplingRateQualityComboBox->findData("automatic"));
    }

    void save()
    {
        QString stereoMode = configDialogSamplingRateQualityComboBox->itemData(configDialogSamplingRateQualityComboBox->currentIndex()).toString();
        plugin->setStereoMode(stereoMode);
        this->deleteLater();
    }
};

soundkonverter_codec_lame::soundkonverter_codec_lame(QObject *parent, const KPluginMetaData &metadata, const QVariantList &args)
    : CodecPlugin(parent)
{
    Q_UNUSED(args)

    binaries["lame"] = "";

    allCodecs += "mp3";
    allCodecs += "mp2";
    allCodecs += "wav";

    KSharedConfig::Ptr conf = KSharedConfig::openConfig();
    KConfigGroup group;

    group = conf->group("Plugin-" + name());
    configVersion = group.readEntry("configVersion", 0);
    stereoMode = group.readEntry("stereoMode", "automatic");
}

soundkonverter_codec_lame::~soundkonverter_codec_lame()
{
}

QString soundkonverter_codec_lame::name() const
{
    return global_plugin_name;
}

QList<ConversionPipeTrunk> soundkonverter_codec_lame::codecTable()
{
    QList<ConversionPipeTrunk> table;
    ConversionPipeTrunk newTrunk;

    newTrunk.codecFrom = "wav";
    newTrunk.codecTo = "mp3";
    newTrunk.rating = 100;
    newTrunk.enabled = (binaries["lame"] != "");
    newTrunk.problemInfo = standardMessage("encode_codec,backend", "mp3", "lame") + "\n" + standardMessage("install_patented_backend", "lame");
    newTrunk.data.hasInternalReplayGain = false;
    table.append(newTrunk);

    newTrunk.codecFrom = "mp3";
    newTrunk.codecTo = "wav";
    newTrunk.rating = 100;
    newTrunk.enabled = (binaries["lame"] != "");
    newTrunk.problemInfo = standardMessage("decode_codec,backend", "mp3", "lame") + "\n" + standardMessage("install_patented_backend", "lame");
    newTrunk.data.hasInternalReplayGain = false;
    table.append(newTrunk);

    newTrunk.codecFrom = "mp3";
    newTrunk.codecTo = "mp3";
    newTrunk.rating = 100;
    newTrunk.enabled = (binaries["lame"] != "");
    newTrunk.problemInfo = standardMessage("transcode_codec,backend", "mp3", "lame") + "\n" + standardMessage("install_patented_backend", "lame");
    newTrunk.data.hasInternalReplayGain = false;
    table.append(newTrunk);

    newTrunk.codecFrom = "mp2";
    newTrunk.codecTo = "wav";
    newTrunk.rating = 70;
    newTrunk.enabled = (binaries["lame"] != "");
    newTrunk.problemInfo = standardMessage("decode_codec,backend", "mp2", "lame") + "\n" + standardMessage("install_patented_backend", "lame");
    newTrunk.data.hasInternalReplayGain = false;
    table.append(newTrunk);

    newTrunk.codecFrom = "mp2";
    newTrunk.codecTo = "mp3";
    newTrunk.rating = 70;
    newTrunk.enabled = (binaries["lame"] != "");
    newTrunk.problemInfo = standardMessage("transcode_codec,backend", "mp2/mp3", "lame") + "\n" + standardMessage("install_patented_backend", "lame");
    newTrunk.data.hasInternalReplayGain = false;
    table.append(newTrunk);

    return table;
}

bool soundkonverter_codec_lame::isConfigSupported(ActionType action, const QString &codecName)
{
    Q_UNUSED(action)
    Q_UNUSED(codecName)

    return true;
}

void soundkonverter_codec_lame::showConfigDialog(ActionType action, const QString &codecName, QWidget *parent)
{
    Q_UNUSED(action)
    Q_UNUSED(codecName)

    if (!configDialog.data())
        configDialog = new ConfigDialog(this, parent);

    if (auto dialog = dynamic_cast<ConfigDialog *>(configDialog.data()))
        dialog->setStereoMode(stereoMode);

    configDialog.data()->show();
}

void soundkonverter_codec_lame::setStereoMode(QString stereoMode)
{
    this->stereoMode = stereoMode;
    KSharedConfig::Ptr conf = KSharedConfig::openConfig();
    KConfigGroup group;

    group = conf->group("Plugin-" + name());
    group.writeEntry("stereoMode", stereoMode);
}

bool soundkonverter_codec_lame::hasInfo()
{
    return true;
}

void soundkonverter_codec_lame::showInfo(QWidget *parent)
{
    QMessageBox::information(parent,
                             i18n("About %1", name()),
                             i18n("LAME is a free high quality MP3 encoder.\nYou can get it at: http://lame.sourceforge.net"));
}

CodecWidget *soundkonverter_codec_lame::newCodecWidget()
{
    LameCodecWidget *widget = new LameCodecWidget();
    return qobject_cast<CodecWidget *>(widget);
}

int soundkonverter_codec_lame::convert(const QUrl &inputFile,
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
    connect(newItem->process, &QProcess::readyRead, this, &soundkonverter_codec_lame::processOutput);
    connect(newItem->process, &QProcess::finished, this, &soundkonverter_codec_lame::processExit);

    newItem->process->clearProgram();
    newItem->process->setShellCommand(command.join(" "));
    newItem->process->start();

    logCommand(newItem->id, command.join(" "));

    backendItems.append(newItem);
    return newItem->id;
}

QStringList soundkonverter_codec_lame::convertCommand(const QUrl &inputFile,
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

    if (inputFile.isEmpty())
        return QStringList();

    QStringList command;
    const ConversionOptions *conversionOptions = _conversionOptions;
    const LameConversionOptions *lameConversionOptions = 0;
    if (conversionOptions->pluginName == name()) {
        lameConversionOptions = dynamic_cast<const LameConversionOptions *>(conversionOptions);
    }

    if (outputCodec == "mp3") {
        command += binaries["lame"];
        command += "--nohist";
        command += "--pad-id3v2";
        if (conversionOptions->pluginName == name()) {
            command += "-q";
            command += QString::number((int)conversionOptions->compressionLevel);
        }
        //         if( conversionOptions->replaygain && replayGain )
        //         {
        //             command += "--replaygain-accurate";
        //         }
        //         else
        //         {
        //             command += "--noreplaygain";
        //         }
        if (conversionOptions->pluginName != name() || !conversionOptions->cmdArguments.contains("replaygain")) {
            command += "--noreplaygain";
        }
        if (lameConversionOptions && lameConversionOptions->data.preset != LameConversionOptions::Data::UserDefined) {
            command += "--preset";
            if (lameConversionOptions->data.presetFast) {
                command += "fast";
            }
            if (lameConversionOptions->data.preset == LameConversionOptions::Data::Medium) {
                command += "medium";
            } else if (lameConversionOptions->data.preset == LameConversionOptions::Data::Standard) {
                command += "standard";
            } else if (lameConversionOptions->data.preset == LameConversionOptions::Data::Extreme) {
                command += "extreme";
            } else if (lameConversionOptions->data.preset == LameConversionOptions::Data::Insane) {
                command += "insane";
            } else if (lameConversionOptions->data.preset == LameConversionOptions::Data::SpecifyBitrate) {
                if (lameConversionOptions->data.presetBitrateCbr) {
                    command += "cbr";
                }
                command += QString::number(lameConversionOptions->data.presetBitrate);
            }
        } else {
            if (conversionOptions->qualityMode == ConversionOptions::Quality) {
                if (conversionOptions->pluginName != name() || !conversionOptions->cmdArguments.contains("--vbr-old")) {
                    command += "--vbr-new";
                }
                command += "-V";
                command += QString::number(conversionOptions->quality);
            } else if (conversionOptions->qualityMode == ConversionOptions::Bitrate) {
                if (conversionOptions->bitrateMode == ConversionOptions::Abr) {
                    command += "--abr";
                    command += QString::number(conversionOptions->bitrate);
                } else if (conversionOptions->bitrateMode == ConversionOptions::Cbr) {
                    command += "--cbr";
                    command += "-b";
                    command += QString::number(conversionOptions->bitrate);
                }
            }
        }
        if (stereoMode != "automatic") {
            command += "-m";
            if (stereoMode == "joint stereo") {
                command += "j";
            } else if (stereoMode == "simple stereo") {
                command += "s";
            } else if (stereoMode == "forced joint stereo") {
                command += "f";
            } else if (stereoMode == "dual mono") {
                command += "d";
            }
        }
        if (conversionOptions->pluginName == name()) {
            command += conversionOptions->cmdArguments;
        }
        command += "\"" + escapeUrl(inputFile) + "\"";
        command += "\"" + escapeUrl(outputFile) + "\"";
    } else {
        command += binaries["lame"];
        command += "--decode";
        command += "\"" + escapeUrl(inputFile) + "\"";
        command += "\"" + escapeUrl(outputFile) + "\"";
    }

    return command;
}

float soundkonverter_codec_lame::parseOutput(const QString &output)
{
    // decoding
    // Frame#  1398/8202   256 kbps  L  R (...)

    // encoding
    // \r  3600/3696   (97%)|    0:05/    0:05|    0:05/    0:05|   18.190x|    0:00

    QString data = output;
    QString frame, count;

    if (output.contains("Frame#")) {
        data.remove(0, data.indexOf("Frame#") + 7);
        frame = data.left(data.indexOf("/"));
        data.remove(0, data.indexOf("/") + 1);
        count = data.left(data.indexOf(" "));
        return frame.toFloat() / count.toFloat() * 100.0f;
    }
    if (output.contains("%")) {
        frame = data.left(data.indexOf("/"));
        frame.remove(0, frame.lastIndexOf(" ") + 1);
        data.remove(0, data.indexOf("/") + 1);
        count = data.left(data.indexOf(" "));
        return frame.toFloat() / count.toFloat() * 100.0f;
    }
    /*if( output.contains("%") )
    {
        data.remove( 0, data.indexOf("(")+1 );
        data.remove( data.indexOf("%"), data.length()-data.indexOf("%") );
        return data.toFloat();
    }*/

    return -1;
}

ConversionOptions *soundkonverter_codec_lame::conversionOptionsFromXml(QDomElement conversionOptions, QList<QDomElement> *filterOptionsElements)
{
    LameConversionOptions *options = new LameConversionOptions();
    options->fromXml(conversionOptions, filterOptionsElements);
    return options;
}

K_PLUGIN_FACTORY_WITH_JSON(soundkonverter_codec_lameFactory, "soundkonverter_codec_lame.json", registerPlugin<soundkonverter_codec_lame>();)

#include "soundkonverter_codec_lame.moc"
