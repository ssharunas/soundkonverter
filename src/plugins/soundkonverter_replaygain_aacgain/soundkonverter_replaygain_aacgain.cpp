#include "aacreplaygainglobal.h"

#include "soundkonverter_replaygain_aacgain.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPageDialog>
#include <KSharedConfig>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

class ConfigDialog : public KPageDialog
{
public:
    explicit ConfigDialog(soundkonverter_replaygain_aacgain *plugin, QWidget *parent)
        : KPageDialog(parent)
        , plugin(plugin)
    {
        setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
        setWindowTitle(i18n("Configure %1", plugin->name()));

        QWidget *configDialogWidget = new QWidget(this);
        QVBoxLayout *configDialogBox = new QVBoxLayout(configDialogWidget);

        QHBoxLayout *configDialogBox1 = new QHBoxLayout();
        QLabel *configDialogTagModeLabel = new QLabel(i18n("Use tag format:"), configDialogWidget);
        configDialogBox1->addWidget(configDialogTagModeLabel);
        configDialogTagModeComboBox = new QComboBox(configDialogWidget);
        configDialogTagModeComboBox->addItem("APE");
        configDialogTagModeComboBox->addItem("ID3v2");
        configDialogBox1->addWidget(configDialogTagModeComboBox);
        configDialogBox->addLayout(configDialogBox1);

        QHBoxLayout *configDialogBox3 = new QHBoxLayout();
        QLabel *configDialogGainAdjustmentLabel = new QLabel(i18n("Adjust gain:"), configDialogWidget);
        configDialogBox3->addWidget(configDialogGainAdjustmentLabel);
        configDialogGainAdjustmentSpinBox = new QDoubleSpinBox(configDialogWidget);
        configDialogGainAdjustmentSpinBox->setRange(-99, 99);
        configDialogGainAdjustmentSpinBox->setSuffix(" " + i18nc("decibel", "dB"));
        configDialogGainAdjustmentSpinBox->setToolTip(i18n("Lower or raise the suggested gain"));
        configDialogBox3->addWidget(configDialogGainAdjustmentSpinBox);
        configDialogBox->addLayout(configDialogBox3);

        QHBoxLayout *configDialogBox2 = new QHBoxLayout();
        configDialogModifyAudioStreamCheckBox = new QCheckBox(i18n("Modify audio stream"), configDialogWidget);
        configDialogModifyAudioStreamCheckBox->setToolTip(
            i18n("Write gain adjustments directly into the encoded data. That way the adjustment works with all mp3 players.\nUndoing the changes is still "
                 "possible since correction data will be written as well."));
        configDialogBox2->addWidget(configDialogModifyAudioStreamCheckBox);
        configDialogBox->addLayout(configDialogBox2);

        connect(this, &ConfigDialog::accepted, this, &ConfigDialog::save);
        connect(buttonBox()->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &ConfigDialog::resetDefault);

        this->addPage(configDialogWidget, "");
    }

    void show(int tagMode, bool modifyAudioStream, double gainAdjustment)
    {
        configDialogTagModeComboBox->setCurrentIndex(tagMode);
        configDialogModifyAudioStreamCheckBox->setChecked(modifyAudioStream);
        configDialogGainAdjustmentSpinBox->setValue(gainAdjustment);
        this->KPageDialog::show();
    }

private:
    QComboBox *configDialogTagModeComboBox;
    QCheckBox *configDialogModifyAudioStreamCheckBox;
    QDoubleSpinBox *configDialogGainAdjustmentSpinBox;
    soundkonverter_replaygain_aacgain *plugin;

    void resetDefault()
    {
        configDialogTagModeComboBox->setCurrentIndex(0);
        configDialogModifyAudioStreamCheckBox->setChecked(false);
        configDialogGainAdjustmentSpinBox->setValue(0.0);
    }

    void save()
    {
        int tagMode = configDialogTagModeComboBox->currentIndex();
        bool modifyAudioStream = configDialogModifyAudioStreamCheckBox->isChecked();
        double gainAdjustment = configDialogGainAdjustmentSpinBox->value();

        plugin->setValues(tagMode, modifyAudioStream, gainAdjustment);
    }
};

AacGainPluginItem::AacGainPluginItem(QObject *parent)
    : ReplayGainPluginItem(parent)
{
}

AacGainPluginItem::~AacGainPluginItem()
{
}

soundkonverter_replaygain_aacgain::soundkonverter_replaygain_aacgain(QObject *parent, const QVariantList &args)
    : ReplayGainPlugin(parent)
{
    Q_UNUSED(args)

    binaries["aacgain"] = "";

    allCodecs += "m4v";
    allCodecs += "mp3";

    KSharedConfig::Ptr conf = KSharedConfig::openConfig();
    KConfigGroup group;

    group = conf->group("Plugin-" + name());
    tagMode = group.readEntry("tagMode", 0);
    modifyAudioStream = group.readEntry("modifyAudioStream", false);
    gainAdjustment = group.readEntry("gainAdjustment", 0.0);
}

soundkonverter_replaygain_aacgain::~soundkonverter_replaygain_aacgain()
{
}

QString soundkonverter_replaygain_aacgain::name() const
{
    return global_plugin_name;
}

QList<ReplayGainPipe> soundkonverter_replaygain_aacgain::codecTable()
{
    QList<ReplayGainPipe> table;
    ReplayGainPipe newPipe;

    newPipe.codecName = "m4a/aac";
    newPipe.rating = 100;
    newPipe.enabled = (binaries["aacgain"] != "");
    newPipe.problemInfo = standardMessage("replygain_codec,backend", "m4a/aac", "aacgain") + "\n" + standardMessage("install_patented_backend", "aacgain");
    table.append(newPipe);

    newPipe.codecName = "mp3";
    newPipe.rating = 95;
    newPipe.enabled = (binaries["aacgain"] != "");
    newPipe.problemInfo = standardMessage("replygain_codec,backend", "mp3", "aacgain") + "\n" + standardMessage("install_patented_backend", "aacgain");
    table.append(newPipe);

    return table;
}

bool soundkonverter_replaygain_aacgain::isConfigSupported(ActionType action, const QString &codecName)
{
    Q_UNUSED(action)
    Q_UNUSED(codecName)

    return true;
}

void soundkonverter_replaygain_aacgain::showConfigDialog(ActionType action, const QString &codecName, QWidget *parent)
{
    Q_UNUSED(action)
    Q_UNUSED(codecName)

    if (!configDialog.data())
        configDialog = new ConfigDialog(this, parent);

    if (auto dialog = dynamic_cast<ConfigDialog *>(configDialog.data()))
        dialog->show(tagMode, modifyAudioStream, gainAdjustment);
}

void soundkonverter_replaygain_aacgain::setValues(int tagMode, bool modifyAudioStream, double gainAdjustment)
{
    KSharedConfig::Ptr conf = KSharedConfig::openConfig();
    KConfigGroup group;

    group = conf->group("Plugin-" + name());
    group.writeEntry("tagMode", tagMode);
    group.writeEntry("modifyAudioStream", modifyAudioStream);
    group.writeEntry("gainAdjustment", gainAdjustment);
}

bool soundkonverter_replaygain_aacgain::hasInfo()
{
    return false;
}

void soundkonverter_replaygain_aacgain::showInfo(QWidget *parent)
{
    Q_UNUSED(parent)
}

int soundkonverter_replaygain_aacgain::apply(const QList<QUrl> &fileList, ReplayGainPlugin::ApplyMode mode)
{
    if (fileList.empty())
        return BackendPlugin::UnknownError;

    AacGainPluginItem *newItem = new AacGainPluginItem(this);
    newItem->id = lastId++;
    newItem->process = new KProcess(newItem);
    newItem->process->setOutputChannelMode(KProcess::MergedChannels);
    connect(newItem->process, &QIODevice::readyRead, this, &soundkonverter_replaygain_aacgain::processOutput);

    QStringList command;
    command += binaries["aacgain"];
    if (mode == ReplayGainPlugin::Add) {
        command += "-k";
        if (modifyAudioStream)
            command += "-a";

        connect(newItem->process, &KProcess::finished, this, &soundkonverter_replaygain_aacgain::undoProcessExit);
    } else if (mode == ReplayGainPlugin::Force) {
        command += "-k";
        if (modifyAudioStream) {
            command += "-a";
        }
        command += "-s";
        command += "r";
        connect(newItem->process, &KProcess::finished, this, &soundkonverter_replaygain_aacgain::undoProcessExit);
    } else {
        command += "-u";
        connect(newItem->process, &KProcess::finished, this, &soundkonverter_replaygain_aacgain::undoProcessExit);
        newItem->undoFileList = fileList;
    }
    if (gainAdjustment != 0) {
        command += "-d";
        command += QString::number(gainAdjustment);
    }
    if (mode == ReplayGainPlugin::Add || mode == ReplayGainPlugin::Force) {
        if (tagMode == 0) {
            // APE tags
            command += "-s";
            command += "a";
        } else {
            // ID3v2 tags
            command += "-s";
            command += "i";
        }
    }
    foreach (const QUrl &file, fileList) {
        command += "\"" + escapeUrl(file) + "\"";
    }

    newItem->process->clearProgram();
    newItem->process->setShellCommand(command.join(" "));
    newItem->process->start();

    logCommand(newItem->id, command.join(" "));

    backendItems.append(newItem);
    return newItem->id;
}

void soundkonverter_replaygain_aacgain::undoProcessExit(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    AacGainPluginItem *item = 0;

    for (int i = 0; i < backendItems.size(); i++) {
        if (backendItems.at(i)->process == QObject::sender()) {
            item = qobject_cast<AacGainPluginItem *>(backendItems.at(i));
            break;
        }
    }

    if (!item)
        return;

    if (item->undoFileList.count() <= 0)
        return;

    if (item->process)
        item->process->deleteLater();

    item->process = new KProcess(item);
    item->process->setOutputChannelMode(KProcess::MergedChannels);
    connect(item->process, &QProcess::readyRead, this, &soundkonverter_replaygain_aacgain::processOutput);
    connect(item->process, &QProcess::finished, this, &soundkonverter_replaygain_aacgain::processExit);

    QStringList command;
    command += binaries["aacgain"];
    // APE tags
    command += "-s";
    command += "a";
    // ID3v2 tags
    command += "-s";
    command += "i";
    // delete tags
    command += "-s";
    command += "d";
    foreach (const QUrl &file, item->undoFileList) {
        command += "\"" + escapeUrl(file) + "\"";
    }

    item->process->clearProgram();
    item->process->setShellCommand(command.join(" "));
    item->process->start();

    logCommand(item->id, command.join(" "));
}

float soundkonverter_replaygain_aacgain::parseOutput(const QString &output)
{
    //  9% of 45218064 bytes analyzed
    // [1/10] 32% of 13066690 bytes analyzed

    float progress = -1.0f;

    QRegularExpression reg1("\\[(\\d+)/(\\d+)\\] (\\d+)%");
    QRegularExpression reg2("(\\d+)%");

    QRegularExpressionMatch match = reg1.matchView(output);
    if (match.hasMatch()) {
        float fraction = 1.0f / match.capturedView(2).toInt();
        progress = 100 * ((float)match.capturedView(1).toInt() - 1) * fraction + (float)match.capturedView(3).toInt() * fraction;
    } else {
        match = reg2.matchView(output);

        if (match.hasMatch())
            progress = (float)match.capturedView(1).toInt();
    }

    if (progress < 0) {
        // Applying mp3 gain change of -6 to /home/user/file.mp3...
        // Undoing mp3gain changes (6,6) to /home/user/file.mp3...
        // Deleting tag info of /home/user/file.mp3...
        QRegularExpression reg3("[Applying mp3 gain change|Undoing mp3gain changes|Deleting tag info]");
        match = reg3.matchView(output);

        if (match.hasMatch())
            progress = 0;
    }

    return progress;
}

K_PLUGIN_FACTORY_WITH_JSON(soundkonverter_replaygain_aacgainFactory, "soundkonverter_replaygain_aacgain.json", registerPlugin<soundkonverter_replaygain_aacgain>();)

#include "soundkonverter_replaygain_aacgain.moc"
