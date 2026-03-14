
#include "mp3replaygainglobal.h"

#include "soundkonverter_replaygain_mp3gain.h"

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
    explicit ConfigDialog(soundkonverter_replaygain_mp3gain *plugin, QWidget *parent)
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
    soundkonverter_replaygain_mp3gain *plugin;

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

Mp3GainPluginItem::Mp3GainPluginItem(QObject *parent)
    : ReplayGainPluginItem(parent)
{
}

Mp3GainPluginItem::~Mp3GainPluginItem()
{
}

soundkonverter_replaygain_mp3gain::soundkonverter_replaygain_mp3gain(QObject *parent, const QVariantList &args)
    : ReplayGainPlugin(parent)
{
    Q_UNUSED(args)

    binaries["mp3gain"] = "";

    allCodecs += "mp3";

    KSharedConfig::Ptr conf = KSharedConfig::openConfig();
    KConfigGroup group;

    group = conf->group("Plugin-" + name());
    tagMode = group.readEntry("tagMode", 0);
    modifyAudioStream = group.readEntry("modifyAudioStream", false);
    gainAdjustment = group.readEntry("gainAdjustment", 0.0);
}

soundkonverter_replaygain_mp3gain::~soundkonverter_replaygain_mp3gain()
{
}

QString soundkonverter_replaygain_mp3gain::name() const
{
    return global_plugin_name;
}

QList<ReplayGainPipe> soundkonverter_replaygain_mp3gain::codecTable()
{
    QList<ReplayGainPipe> table;
    ReplayGainPipe newPipe;

    newPipe.codecName = "mp3";
    newPipe.rating = 100;
    newPipe.enabled = (binaries["mp3gain"] != "");
    newPipe.problemInfo = standardMessage("replygain_codec,backend", "mp3", "mp3gain") + "\n" + standardMessage("install_patented_backend", "mp3gain");
    table.append(newPipe);

    return table;
}

bool soundkonverter_replaygain_mp3gain::isConfigSupported(ActionType action, const QString &codecName)
{
    Q_UNUSED(action)
    Q_UNUSED(codecName)

    return true;
}

void soundkonverter_replaygain_mp3gain::showConfigDialog(ActionType action, const QString &codecName, QWidget *parent)
{
    Q_UNUSED(action)
    Q_UNUSED(codecName)

    if (!configDialog.data())
        configDialog = new ConfigDialog(this, parent);

    if (auto dialog = dynamic_cast<ConfigDialog *>(configDialog.data()))
        dialog->show(tagMode, modifyAudioStream, gainAdjustment);
}

void soundkonverter_replaygain_mp3gain::setValues(int tagMode, bool modifyAudioStream, double gainAdjustment)
{
    KSharedConfig::Ptr conf = KSharedConfig::openConfig();
    KConfigGroup group;

    group = conf->group("Plugin-" + name());
    group.writeEntry("tagMode", tagMode);
    group.writeEntry("modifyAudioStream", modifyAudioStream);
    group.writeEntry("gainAdjustment", gainAdjustment);
}

bool soundkonverter_replaygain_mp3gain::hasInfo()
{
    return false;
}

void soundkonverter_replaygain_mp3gain::showInfo(QWidget *parent)
{
    Q_UNUSED(parent)
}

int soundkonverter_replaygain_mp3gain::apply(const QList<QUrl> &fileList, ReplayGainPlugin::ApplyMode mode)
{
    if (fileList.count() <= 0)
        return BackendPlugin::UnknownError;

    Mp3GainPluginItem *newItem = new Mp3GainPluginItem(this);
    newItem->id = lastId++;
    newItem->process = new KProcess(newItem);
    newItem->process->setOutputChannelMode(KProcess::MergedChannels);
    connect(newItem->process, &KProcess::readyRead, this, &soundkonverter_replaygain_mp3gain::processOutput);

    QStringList command;
    command += binaries["mp3gain"];
    if (mode == ReplayGainPlugin::Add) {
        command += "-k";
        if (modifyAudioStream) {
            command += "-a";
        }
        connect(newItem->process, &KProcess::finished, this, &soundkonverter_replaygain_mp3gain::processExit);
    } else if (mode == ReplayGainPlugin::Force) {
        command += "-k";
        if (modifyAudioStream) {
            command += "-a";
        }
        command += "-s";
        command += "r";
        connect(newItem->process, &KProcess::finished, this, &soundkonverter_replaygain_mp3gain::processExit);
    } else {
        command += "-u";
        connect(newItem->process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(undoProcessExit(int, QProcess::ExitStatus)));
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

void soundkonverter_replaygain_mp3gain::undoProcessExit(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    Mp3GainPluginItem *item = 0;

    for (int i = 0; i < backendItems.size(); i++) {
        if (backendItems.at(i)->process == QObject::sender()) {
            item = qobject_cast<Mp3GainPluginItem *>(backendItems.at(i));
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
    connect(item->process, &KProcess::readyRead, this, &soundkonverter_replaygain_mp3gain::processOutput);
    connect(item->process, &KProcess::finished, this, &soundkonverter_replaygain_mp3gain::processExit);

    QStringList command;
    command += binaries["mp3gain"];
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

float soundkonverter_replaygain_mp3gain::parseOutput(const QString &output)
{
    //  9% of 45218064 bytes analyzed
    // [1/10] 32% of 13066690 bytes analyzed

    float progress = -1.0f;

    QRegularExpression reg1("\\[(\\d+)/(\\d+)\\] (\\d+)%");
    QRegularExpressionMatch match = reg1.matchView(output);

    if (match.hasMatch()) {
        float fraction = 1.0f / match.capturedView(2).toInt();
        progress = 100 * (match.capturedView(1).toInt() - 1) * fraction + match.capturedView(3).toInt() * fraction;
    } else {
        QRegularExpression reg2("(\\d+)%");
        match = reg2.matchView(output);

        if (match.hasMatch())
            progress = match.capturedView(1).toInt();
    }

    if (progress == -1) {
        // Applying mp3 gain change of -6 to /home/user/file.mp3...
        // Undoing mp3gain changes (6,6) to /home/user/file.mp3...
        // Deleting tag info of /home/user/file.mp3...
        QRegularExpression reg3("[Applying mp3 gain change|Undoing mp3gain changes|Deleting tag info]");
        match = reg3.matchView(output);
        if (match.hasMatch())
            progress = 0.0f;
    }

    return progress;
}

K_PLUGIN_FACTORY_WITH_JSON(soundkonverter_replaygain_mp3gainFactory, "soundkonverter_replaygain_mp3gain.json", registerPlugin<soundkonverter_replaygain_mp3gain>();)

#include "soundkonverter_replaygain_mp3gain.moc"
