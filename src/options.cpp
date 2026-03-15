#include "options.h"
#include "config.h"
#include "optionsdetailed.h"
#include "optionssimple.h"
#include "pluginloader.h"

#include <KLocalizedString>
#include <QLayout>
#include <QLocale>
#include <QTabWidget>

Options::Options(Config *_config, const QString &text, QWidget *parent)
    : QWidget(parent)
    , config(_config)
{
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    tab = new QTabWidget(this);
    gridLayout->addWidget(tab, 0, 0);
    connect(tab, &QTabWidget::currentChanged, this, &Options::tabChanged);

    optionsSimple = new OptionsSimple(config, text, this);
    connect(optionsSimple, &OptionsSimple::optionsChanged, this, &Options::simpleOptionsChanged);
    connect(optionsSimple->outputDirectory, &OutputDirectory::modeChanged, this, &Options::simpleOutputDirectoryModeChanged);
    connect(optionsSimple->outputDirectory, &OutputDirectory::directoryChanged, this, &Options::simpleOutputDirectoryChanged);

    optionsDetailed = new OptionsDetailed(config, this);
    connect(optionsDetailed->outputDirectory, &OutputDirectory::modeChanged, this, &Options::detailedOutputDirectoryModeChanged);
    connect(optionsDetailed, &OptionsDetailed::currentDataRateChanged, optionsSimple, &OptionsSimple::currentDataRateChanged);

    connect(optionsDetailed, &OptionsDetailed::customProfilesEdited, optionsSimple, &OptionsSimple::updateProfiles);
    connect(optionsSimple, &OptionsSimple::customProfilesEdited, optionsDetailed, &OptionsDetailed::updateProfiles);

    optionsSimple->init();
    optionsDetailed->init();

    QString format;
    const QStringList formats = config->pluginLoader()->formatList(
        PluginLoader::Encode,
        PluginLoader::CompressionType(PluginLoader::InferiorQuality | PluginLoader::Lossy | PluginLoader::Lossless | PluginLoader::Hybrid));
    if (config->data.general.defaultFormat == i18n("Last used") || config->data.general.defaultFormat == "Last used") {
        format = config->data.general.lastFormat;
    } else {
        format = config->data.general.defaultFormat;
    }
    if (!formats.contains(format))
        format.clear();

    if (format.isEmpty() && formats.count() > 0) {
        if (formats.contains(config->data.general.lastFormat))
            format = config->data.general.lastFormat;
        else
            format = formats.at(0);
    }
    optionsDetailed->setCurrentFormat(format);

    QString profile;
    if (config->data.general.defaultProfile == i18n("Last used") || config->data.general.defaultProfile == "Last used") {
        profile = "soundkonverter_last_used";
    } else {
        profile = config->data.general.defaultProfile;
    }
    if (profile.isEmpty())
        profile = i18n("High");

    optionsDetailed->setCurrentProfile(profile);

    const int startTab = (config->data.general.startTab == 0) ? config->data.general.lastTab : config->data.general.startTab - 1;

    tab->addTab(optionsSimple, i18n("Simple"));
    tab->addTab(optionsDetailed, i18n("Detailed"));

    tab->setCurrentIndex(startTab);
}

Options::~Options()
{
}

ConversionOptions *Options::currentConversionOptions()
{
    return optionsDetailed->currentConversionOptions();
}

bool Options::setCurrentConversionOptions(const ConversionOptions *conversionOptions)
{
    const bool success = optionsDetailed->setCurrentConversionOptions(conversionOptions);
    tabChanged(0); // update optionsSimple
    return success;
}

void Options::simpleOutputDirectoryModeChanged(const OutputDirectory::Mode mode)
{
    if (optionsDetailed && optionsDetailed->outputDirectory)
        optionsDetailed->outputDirectory->setMode(mode);

    config->data.general.lastOutputDirectoryMode = mode;
}

void Options::simpleOutputDirectoryChanged(const QString &directory)
{
    if (optionsDetailed && optionsDetailed->outputDirectory)
        optionsDetailed->outputDirectory->setDirectory(directory);
}

void Options::simpleOptionsChanged()
{
    optionsDetailed->setCurrentFormat(optionsSimple->currentFormat());
    optionsDetailed->setCurrentProfile(optionsSimple->currentProfile());
    optionsDetailed->resetFilterOptions();
    optionsDetailed->setReplayGainChecked(optionsSimple->isReplayGainChecked());
    QString toolTip;
    const bool replaygainEnabled = optionsDetailed->isReplayGainEnabled(&toolTip);
    optionsSimple->setReplayGainEnabled(replaygainEnabled, toolTip);
}

void Options::detailedOutputDirectoryModeChanged(const int mode)
{
    config->data.general.lastOutputDirectoryMode = mode;
}

void Options::tabChanged(const int pageIndex)
{
    if (pageIndex == 0) {
        // NOTE prevent signals from firing back
        disconnect(optionsSimple, &OptionsSimple::optionsChanged, 0, 0);
        disconnect(optionsSimple->outputDirectory, &OutputDirectory::modeChanged, 0, 0);
        disconnect(optionsSimple->outputDirectory, &OutputDirectory::directoryChanged, 0, 0);

        optionsSimple->updateProfiles();
        optionsSimple->setCurrentProfile(optionsDetailed->currentProfile());
        optionsSimple->setCurrentFormat(optionsDetailed->currentFormat());
        QString toolTip;
        const bool replaygainEnabled = optionsDetailed->isReplayGainEnabled(&toolTip);
        const bool replaygainChecked = optionsDetailed->isReplayGainChecked();
        optionsSimple->setReplayGainEnabled(replaygainEnabled, toolTip);
        optionsSimple->setReplayGainChecked(replaygainChecked);
        optionsSimple->setCurrentPlugin(optionsDetailed->getCurrentPlugin());

        optionsSimple->outputDirectory->setMode(optionsDetailed->outputDirectory->mode());
        optionsSimple->outputDirectory->setDirectory(optionsDetailed->outputDirectory->directory());

        connect(optionsSimple, &OptionsSimple::optionsChanged, this, &Options::simpleOptionsChanged);
        connect(optionsSimple->outputDirectory, &OutputDirectory::modeChanged, this, &Options::simpleOutputDirectoryModeChanged);
        connect(optionsSimple->outputDirectory, &OutputDirectory::directoryChanged, this, &Options::simpleOutputDirectoryChanged);
    }

    config->data.general.lastTab = tab->currentIndex();
}

void Options::setProfile(const QString &profile)
{
    optionsSimple->setCurrentProfile(profile);
    simpleOptionsChanged();
}

void Options::setFormat(const QString &format)
{
    optionsSimple->setCurrentFormat(format);
    simpleOptionsChanged();
}

void Options::setOutputDirectoryMode(OutputDirectory::Mode mode)
{
    QString directory;
    optionsSimple->setCurrentOutputDirectoryMode(mode);
    if (mode == (int)OutputDirectory::Specify)
        directory = config->data.general.specifyOutputDirectory;
    else if (mode == (int)OutputDirectory::Source)
        directory = "";
    else if (mode == (int)OutputDirectory::MetaData)
        directory = config->data.general.metaDataOutputDirectory;
    else if (mode == (int)OutputDirectory::CopyStructure)
        directory = config->data.general.copyStructureOutputDirectory;
    optionsSimple->setCurrentOutputDirectory(directory);
    simpleOutputDirectoryModeChanged(mode);
    simpleOutputDirectoryChanged(directory);
}

void Options::setOutputDirectory(const QString &directory)
{
    optionsSimple->setCurrentOutputDirectoryMode(OutputDirectory::Specify);
    optionsSimple->setCurrentOutputDirectory(directory);
    simpleOutputDirectoryModeChanged(OutputDirectory::Specify);
    simpleOutputDirectoryChanged(directory);
}

void Options::accepted()
{
    const OutputDirectory::Mode mode = (OutputDirectory::Mode)config->data.general.lastOutputDirectoryMode;

    if (mode == OutputDirectory::MetaData) {
        const QString path = config->data.general.metaDataOutputDirectory;
        if (config->data.general.lastMetaDataOutputDirectoryPaths.contains(path))
            config->data.general.lastMetaDataOutputDirectoryPaths.removeAll(path);
        else if (config->data.general.lastMetaDataOutputDirectoryPaths.size() >= 5)
            config->data.general.lastMetaDataOutputDirectoryPaths.removeLast();
        config->data.general.lastMetaDataOutputDirectoryPaths.prepend(path);
    } else if (mode == OutputDirectory::Specify) {
        const QString path = config->data.general.specifyOutputDirectory;
        if (config->data.general.lastNormalOutputDirectoryPaths.contains(path))
            config->data.general.lastNormalOutputDirectoryPaths.removeAll(path);
        else if (config->data.general.lastNormalOutputDirectoryPaths.size() >= 5)
            config->data.general.lastNormalOutputDirectoryPaths.removeLast();
        config->data.general.lastNormalOutputDirectoryPaths.prepend(path);
    } else if (mode == OutputDirectory::CopyStructure) {
        const QString path = config->data.general.copyStructureOutputDirectory;
        if (config->data.general.lastNormalOutputDirectoryPaths.contains(path))
            config->data.general.lastNormalOutputDirectoryPaths.removeAll(path);
        else if (config->data.general.lastNormalOutputDirectoryPaths.size() >= 5)
            config->data.general.lastNormalOutputDirectoryPaths.removeLast();
        config->data.general.lastNormalOutputDirectoryPaths.prepend(path);
    }
}
