
#include "normalizefilterglobal.h"

#include "../../core/conversionoptions.h"
#include "normalizefilteroptions.h"
#include "normalizefilterwidget.h"
#include "soundkonverter_filter_normalize.h"

#include <QFile>

soundkonverter_filter_normalize::soundkonverter_filter_normalize(QObject *parent, const QVariantList &args)
    : FilterPlugin(parent)
{
    Q_UNUSED(args)

    binaries["normalize"] = "";

    allCodecs += "wav";
}

soundkonverter_filter_normalize::~soundkonverter_filter_normalize()
{
}

QString soundkonverter_filter_normalize::name() const
{
    return global_plugin_name;
}

QList<ConversionPipeTrunk> soundkonverter_filter_normalize::codecTable()
{
    QList<ConversionPipeTrunk> table;
    ConversionPipeTrunk newTrunk;

    newTrunk.codecFrom = "wav";
    newTrunk.codecTo = "wav";
    newTrunk.rating = 100;
    newTrunk.enabled = (binaries["normalize"] != "");
    newTrunk.problemInfo = standardMessage("filter,backend", "normalize", "normalize") + "\n" + standardMessage("install_opensource_backend", "normalize");
    newTrunk.data.hasInternalReplayGain = false;
    table.append(newTrunk);

    return table;
}

bool soundkonverter_filter_normalize::isConfigSupported(ActionType action, const QString &codecName)
{
    Q_UNUSED(action)
    Q_UNUSED(codecName)

    return false;
}

void soundkonverter_filter_normalize::showConfigDialog(ActionType action, const QString &codecName, QWidget *parent)
{
    Q_UNUSED(action)
    Q_UNUSED(codecName)
    Q_UNUSED(parent)
}

bool soundkonverter_filter_normalize::hasInfo()
{
    return false;
}

void soundkonverter_filter_normalize::showInfo(QWidget *parent)
{
    Q_UNUSED(parent)
}

FilterWidget *soundkonverter_filter_normalize::newFilterWidget()
{
    NormalizeFilterWidget *widget = new NormalizeFilterWidget();
    if (lastUsedFilterOptions)
        widget->setCurrentFilterOptions(lastUsedFilterOptions);

    return qobject_cast<FilterWidget *>(widget);
}

CodecWidget *soundkonverter_filter_normalize::newCodecWidget()
{
    return nullptr;
}

int soundkonverter_filter_normalize::convert(const QUrl &inputFile,
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

    FilterPluginItem *newItem = new FilterPluginItem(this);
    newItem->id = lastId++;
    newItem->process = new KProcess(newItem);
    newItem->process->setOutputChannelMode(KProcess::MergedChannels);

    connect(newItem->process, &QIODevice::readyRead, this, &soundkonverter_filter_normalize::processOutput);
    connect(newItem->process, &QProcess::finished, this, &soundkonverter_filter_normalize::processExit);

    newItem->process->clearProgram();
    newItem->process->setShellCommand(command.join(" "));
    newItem->process->start();

    logCommand(newItem->id, command.join(" "));

    backendItems.append(newItem);
    return newItem->id;
}

QStringList soundkonverter_filter_normalize::convertCommand(const QUrl &inputFile,
                                                            const QUrl &outputFile,
                                                            const QString &inputCodec,
                                                            const QString &outputCodec,
                                                            const ConversionOptions *_conversionOptions,
                                                            TagData *tags,
                                                            bool replayGain)
{
    Q_UNUSED(inputCodec);
    Q_UNUSED(outputCodec);
    Q_UNUSED(tags);
    Q_UNUSED(replayGain);

    if (!_conversionOptions)
        return QStringList();

    if (inputFile.isEmpty() || outputFile.isEmpty())
        return QStringList();

    QStringList command;

    foreach (const FilterOptions *filterOptions, _conversionOptions->filterOptions) {
        if (filterOptions->pluginName == global_plugin_name) {
            const NormalizeFilterOptions *filterOption = dynamic_cast<const NormalizeFilterOptions *>(filterOptions);
            if (filterOption->data.normalize) {
                command += binaries["normalize"];
                command += "\"" + escapeUrl(outputFile) + "\"";

                if (!command.isEmpty())
                    QFile::copy(inputFile.toLocalFile(), outputFile.toLocalFile());
            }
        }
    }

    return command;
}

float soundkonverter_filter_normalize::parseOutput(const QString &output)
{
    Q_UNUSED(output);

    // Computing levels...
    //  aaa.wav           100% done, ETA 00:00:00 (batch 100% done, ETA 00:00:00)
    // Applying adjustment of -4,15dB to aaa.wav...
    //  aaa.wav           100% done, ETA 00:00:00 (batch 100% done, ETA 00:00:00)

    static QRegularExpression re("(\\d+)% done");
    QRegularExpressionMatch match = re.matchView(output);

    if (match.hasMatch())
        return match.capturedView(1).toFloat();

    return -1;
}

FilterOptions *soundkonverter_filter_normalize::filterOptionsFromXml(QDomElement filterOptions)
{
    NormalizeFilterOptions *options = new NormalizeFilterOptions();
    options->fromXml(filterOptions);
    return options;
}

K_PLUGIN_FACTORY_WITH_JSON(soundkonverter_filter_normalizeFactory, "soundkonverter_filter_normalize.json", registerPlugin<soundkonverter_filter_normalize>();)

#include "soundkonverter_filter_normalize.moc"
