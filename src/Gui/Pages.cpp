/*
 * Copyright (c) 2012 Mark Liversedge (liversedge@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "Athlete.h"
#include <QtGui>
#include <QIntValidator>

#include <assert.h>

#include "Pages.h"
#include "Units.h"
#include "Settings.h"
#include "UserMetricParser.h"
#include "Colors.h"
#include "AddDeviceWizard.h"
#include "AddCloudWizard.h"
#include "DeviceTypes.h"
#include "DeviceConfiguration.h"
#include "ColorButton.h"
#include "SpecialFields.h"
#include "DataProcessor.h"
#include "OAuthDialog.h"
#include "RideAutoImportConfig.h"
#include "HelpWhatsThis.h"
#include "GcUpgrade.h"
#include "Dropbox.h"
#include "GoogleDrive.h"
#include "LocalFileStore.h"
#include "Secrets.h"
#include "Utils.h"
#ifdef GC_WANT_PYTHON
#include "PythonEmbed.h"
#include "FixPySettings.h"
#endif
#ifdef GC_HAS_CLOUD_DB
#include "CloudDBUserMetric.h"
#endif
#include "MainWindow.h"
extern ConfigDialog *configdialog_ptr;

//
// Main Config Page - tabs for each sub-page
//
GeneralPage::GeneralPage(Context *context) : context(context)
{
    // layout without too wide margins
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QGridLayout *configLayout = new QGridLayout;
    mainLayout->addLayout(configLayout);
    mainLayout->addStretch();
    setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0,0,0,0);
    configLayout->setSpacing(10 *dpiXFactor);

    //
    // Language Selection
    //
    langLabel = new QLabel(tr("Language"));
    langCombo = new QComboBox();
    langCombo->addItem(tr("English"));
    langCombo->addItem(tr("French"));
    langCombo->addItem(tr("Japanese"));
    langCombo->addItem(tr("Portugese (Brazil)"));
    langCombo->addItem(tr("Italian"));
    langCombo->addItem(tr("German"));
    langCombo->addItem(tr("Russian"));
    langCombo->addItem(tr("Czech"));
    langCombo->addItem(tr("Spanish"));
    langCombo->addItem(tr("Portugese"));
    langCombo->addItem(tr("Chinese (Simplified)"));    
    langCombo->addItem(tr("Chinese (Traditional)"));
    langCombo->addItem(tr("Dutch"));
    langCombo->addItem(tr("Swedish"));

    // Default to system locale
    QVariant lang = appsettings->value(this, GC_LANG, QLocale::system().name());

    if(lang.toString().startsWith("en")) langCombo->setCurrentIndex(0);
    else if(lang.toString().startsWith("fr")) langCombo->setCurrentIndex(1);
    else if(lang.toString().startsWith("ja")) langCombo->setCurrentIndex(2);
    else if(lang.toString().startsWith("pt-br")) langCombo->setCurrentIndex(3);
    else if(lang.toString().startsWith("it")) langCombo->setCurrentIndex(4);
    else if(lang.toString().startsWith("de")) langCombo->setCurrentIndex(5);
    else if(lang.toString().startsWith("ru")) langCombo->setCurrentIndex(6);
    else if(lang.toString().startsWith("cs")) langCombo->setCurrentIndex(7);
    else if(lang.toString().startsWith("es")) langCombo->setCurrentIndex(8);
    else if(lang.toString().startsWith("pt")) langCombo->setCurrentIndex(9);
    else if(lang.toString().startsWith("zh-cn")) langCombo->setCurrentIndex(10);    
    else if (lang.toString().startsWith("zh-tw")) langCombo->setCurrentIndex(11);
    else if (lang.toString().startsWith("nl")) langCombo->setCurrentIndex(12);
    else if (lang.toString().startsWith("sv")) langCombo->setCurrentIndex(13);
    else langCombo->setCurrentIndex(0);

    configLayout->addWidget(langLabel, 0,0, Qt::AlignRight);
    configLayout->addWidget(langCombo, 0,1, Qt::AlignLeft);

    //
    // Units
    //
    QLabel *unitlabel = new QLabel(tr("Unit"));
    unitCombo = new QComboBox();
    unitCombo->addItem(tr("Metric"));
    unitCombo->addItem(tr("Imperial"));

    if (GlobalContext::context()->useMetricUnits)
        unitCombo->setCurrentIndex(0);
    else
        unitCombo->setCurrentIndex(1);

    configLayout->addWidget(unitlabel, 1,0, Qt::AlignRight);
    configLayout->addWidget(unitCombo, 1,1, Qt::AlignLeft);

    metricRunPace = new QCheckBox(tr("Metric Run Pace"), this);
    metricRunPace->setCheckState(appsettings->value(this, GC_PACE, GlobalContext::context()->useMetricUnits).toBool() ? Qt::Checked : Qt::Unchecked);
    configLayout->addWidget(metricRunPace, 2,1, Qt::AlignLeft);

    metricSwimPace = new QCheckBox(tr("Metric Swim Pace"), this);
    metricSwimPace->setCheckState(appsettings->value(this, GC_SWIMPACE, GlobalContext::context()->useMetricUnits).toBool() ? Qt::Checked : Qt::Unchecked);
    configLayout->addWidget(metricSwimPace, 3,1, Qt::AlignLeft);

    //
    // Garmin crap
    //
    // garmin Smart Recording options
    QVariant garminHWMark = appsettings->value(this, GC_GARMIN_HWMARK);
    garminSmartRecord = new QCheckBox(tr("Use Garmin Smart Recording"), this);
    QVariant isGarminSmartRecording = appsettings->value(this, GC_GARMIN_SMARTRECORD, Qt::Checked);
    garminSmartRecord->setCheckState(isGarminSmartRecording.toInt() > 0 ? Qt::Checked : Qt::Unchecked);

    // by default, set the threshold to 25 seconds
    if (garminHWMark.isNull() || garminHWMark.toInt() == 0) garminHWMark.setValue(25);
    QLabel *garminHWLabel = new QLabel(tr("Smart Recording Threshold (secs)"));
    garminHWMarkedit = new QLineEdit(garminHWMark.toString(),this);
    garminHWMarkedit->setInputMask("009");

    configLayout->addWidget(garminSmartRecord, 4,1, Qt::AlignLeft);
    configLayout->addWidget(garminHWLabel, 5,0, Qt::AlignRight);
    configLayout->addWidget(garminHWMarkedit, 5,1, Qt::AlignLeft);

    // Elevation hysterisis  GC_ELEVATION_HYSTERISIS
    QVariant elevationHysteresis = appsettings->value(this, GC_ELEVATION_HYSTERESIS);
    if (elevationHysteresis.isNull() || elevationHysteresis.toFloat() == 0.0)
       elevationHysteresis.setValue(3.0);  // default is 1 meter

    QLabel *hystlabel = new QLabel(tr("Elevation hysteresis (meters)"));
    hystedit = new QLineEdit(elevationHysteresis.toString(),this);
    hystedit->setInputMask("9.00");

    configLayout->addWidget(hystlabel, 6,0, Qt::AlignRight);
    configLayout->addWidget(hystedit, 6,1, Qt::AlignLeft);


    // wbal formula preference
    QLabel *wbalFormLabel = new QLabel(tr("W' bal formula"));
    wbalForm = new QComboBox(this);
    wbalForm->addItem(tr("Differential"));
    wbalForm->addItem(tr("Integral"));
    if (appsettings->value(this, GC_WBALFORM, "diff").toString() == "diff") wbalForm->setCurrentIndex(0);
    else wbalForm->setCurrentIndex(1);

    configLayout->addWidget(wbalFormLabel, 7,0, Qt::AlignRight);
    configLayout->addWidget(wbalForm, 7,1, Qt::AlignLeft);


    //
    // Warn to save on exit
    warnOnExit = new QCheckBox(tr("Warn for unsaved activities on exit"), this);
    warnOnExit->setChecked(appsettings->value(NULL, GC_WARNEXIT, true).toBool());
    configLayout->addWidget(warnOnExit, 8,1, Qt::AlignLeft);

    //
    // Run API web services when running
    //
    int offset=0;
#ifdef GC_WANT_HTTP
    offset += 1;
    startHttp = new QCheckBox(tr("Enable API Web Services"), this);
    startHttp->setChecked(appsettings->value(NULL, GC_START_HTTP, false).toBool());
    configLayout->addWidget(startHttp, 9,1, Qt::AlignLeft);
#endif
#ifdef GC_WANT_R
    embedR = new QCheckBox(tr("Enable R"), this);
    embedR->setChecked(appsettings->value(NULL, GC_EMBED_R, true).toBool());
    configLayout->addWidget(embedR, 9+offset,1, Qt::AlignLeft);
    offset += 1;
    connect(embedR, SIGNAL(stateChanged(int)), this, SLOT(embedRchanged(int)));
#endif

#ifdef GC_WANT_PYTHON
    embedPython = new QCheckBox(tr("Enable Python"), this);
    embedPython->setChecked(appsettings->value(NULL, GC_EMBED_PYTHON, true).toBool());
    configLayout->addWidget(embedPython, 9+offset,1, Qt::AlignLeft);
    connect(embedPython, SIGNAL(stateChanged(int)), this, SLOT(embedPythonchanged(int)));
    offset += 1;
#endif

    opendata = new QCheckBox(tr("Share to the OpenData project"), this);
    QString grant = appsettings->cvalue(context->athlete->cyclist, GC_OPENDATA_GRANTED, "X").toString();
    opendata->setChecked(grant == "Y");
    configLayout->addWidget(opendata, 9+offset,1, Qt::AlignLeft);
    if (grant == "X") opendata->hide();
    offset += 1;

    //
    // Athlete directory (home of athletes)
    //
    QVariant athleteDir = appsettings->value(this, GC_HOMEDIR);
    athleteLabel = new QLabel(tr("Athlete Library"));
    athleteDirectory = new QLineEdit;
    athleteDirectory->setText(athleteDir.toString() == "0" ? "" : athleteDir.toString());
    athleteWAS = athleteDirectory->text(); // remember what we started with ...
    athleteBrowseButton = new QPushButton(tr("Browse"));
    //XXathleteBrowseButton->setFixedWidth(120);

    configLayout->addWidget(athleteLabel, 9 + offset,0, Qt::AlignRight);
    configLayout->addWidget(athleteDirectory, 9 + offset,1);
    configLayout->addWidget(athleteBrowseButton, 9 + offset,2);

    connect(athleteBrowseButton, SIGNAL(clicked()), this, SLOT(browseAthleteDir()));

    //
    // Workout directory (train view)
    //
    QVariant workoutDir = appsettings->value(this, GC_WORKOUTDIR, "");
    // fix old bug..
    if (workoutDir == "0") workoutDir = "";
    workoutLabel = new QLabel(tr("Workout and VideoSync Library"));
    workoutDirectory = new QLineEdit;
    workoutDirectory->setText(workoutDir.toString());
    workoutBrowseButton = new QPushButton(tr("Browse"));
    //XXworkoutBrowseButton->setFixedWidth(120);

    configLayout->addWidget(workoutLabel, 10 + offset,0, Qt::AlignRight);
    configLayout->addWidget(workoutDirectory, 10 + offset,1);
    configLayout->addWidget(workoutBrowseButton, 10 + offset,2);

    connect(workoutBrowseButton, SIGNAL(clicked()), this, SLOT(browseWorkoutDir()));
    offset++;

#ifdef GC_WANT_R
    //
    // R Home directory
    //
    QVariant rDir = appsettings->value(this, GC_R_HOME, "");
    // fix old bug..
    if (rDir == "0") rDir = "";
    rLabel = new QLabel(tr("R Installation Directory"));
    rDirectory = new QLineEdit;
    rDirectory->setText(rDir.toString());
    rBrowseButton = new QPushButton(tr("Browse"));
    //XXrBrowseButton->setFixedWidth(120);

    configLayout->addWidget(rLabel, 10 + offset,0, Qt::AlignRight);
    configLayout->addWidget(rDirectory, 10 + offset,1);
    configLayout->addWidget(rBrowseButton, 10 + offset,2);
    offset++;

    connect(rBrowseButton, SIGNAL(clicked()), this, SLOT(browseRDir()));
#endif
#ifdef GC_WANT_PYTHON
    //
    // Python Home directory
    //
    QVariant pythonDir = appsettings->value(this, GC_PYTHON_HOME, "");
    // fix old bug..
    pythonLabel = new QLabel(tr("Python Home"));
    pythonDirectory = new QLineEdit;
    pythonDirectory->setText(pythonDir.toString());
    pythonBrowseButton = new QPushButton(tr("Browse"));

    configLayout->addWidget(pythonLabel, 10 + offset,0, Qt::AlignRight);
    configLayout->addWidget(pythonDirectory, 10 + offset,1);
    configLayout->addWidget(pythonBrowseButton, 10 + offset,2);
    offset++;

    bool embedPython = appsettings->value(NULL, GC_EMBED_PYTHON, true).toBool();
    embedPythonchanged(embedPython);

    connect(pythonBrowseButton, SIGNAL(clicked()), this, SLOT(browsePythonDir()));
#endif

    // save away initial values
    b4.unit = unitCombo->currentIndex();
    b4.metricRunPace = metricRunPace->isChecked();
    b4.metricSwimPace = metricSwimPace->isChecked();
    b4.hyst = elevationHysteresis.toFloat();
    b4.wbal = wbalForm->currentIndex();
    b4.warn = warnOnExit->isChecked();
#ifdef GC_WANT_HTTP
    b4.starthttp = startHttp->isChecked();
#endif
}

#ifdef GC_WANT_R
void
GeneralPage::embedRchanged(int state)
{
    rBrowseButton->setVisible(state);
    rDirectory->setVisible(state);
    rLabel->setVisible(state);
}
#endif
#ifdef GC_WANT_PYTHON
void
GeneralPage::embedPythonchanged(int state)
{
    pythonBrowseButton->setVisible(state);
    pythonDirectory->setVisible(state);
    pythonLabel->setVisible(state);
}
#endif

qint32
GeneralPage::saveClicked()
{
    // Language
    static const QString langs[] = {
        "en", "fr", "ja", "pt-br", "it", "de", "ru", "cs", "es", "pt", "zh-cn", "zh-tw", "nl", "sv"
    };
    appsettings->setValue(GC_LANG, langs[langCombo->currentIndex()]);

    // Garmin and cranks
    appsettings->setValue(GC_GARMIN_HWMARK, garminHWMarkedit->text().toInt());
    appsettings->setValue(GC_GARMIN_SMARTRECORD, garminSmartRecord->checkState());

    // save on exit
    appsettings->setValue(GC_WARNEXIT, warnOnExit->isChecked());

    // Directories
    appsettings->setValue(GC_WORKOUTDIR, workoutDirectory->text());
    appsettings->setValue(GC_HOMEDIR, athleteDirectory->text());
#ifdef GC_WANT_R
    appsettings->setValue(GC_R_HOME, rDirectory->text());
#endif
#ifdef GC_WANT_PYTHON
    appsettings->setValue(GC_PYTHON_HOME, pythonDirectory->text());
#endif

    // update to reflect the state - if hidden user hasn't been asked yet to
    // its neither set or unset !
    if (!opendata->isHidden()) appsettings->setCValue(context->athlete->cyclist, GC_OPENDATA_GRANTED, opendata->isChecked() ? "Y" : "N");

    // Elevation
    appsettings->setValue(GC_ELEVATION_HYSTERESIS, hystedit->text());

    // wbal formula
    appsettings->setValue(GC_WBALFORM, wbalForm->currentIndex() ? "int" : "diff");

    // Units
    if (unitCombo->currentIndex()==0)
        appsettings->setValue(GC_UNIT, GC_UNIT_METRIC);
    else if (unitCombo->currentIndex()==1)
        appsettings->setValue(GC_UNIT, GC_UNIT_IMPERIAL);

    appsettings->setValue(GC_PACE, metricRunPace->isChecked());
    appsettings->setValue(GC_SWIMPACE, metricSwimPace->isChecked());

#ifdef GC_WANT_HTTP
    // start http
    appsettings->setValue(GC_START_HTTP, startHttp->isChecked());
#endif
#ifdef GC_WANT_R
    appsettings->setValue(GC_EMBED_R, embedR->isChecked());
#endif
#ifdef GC_WANT_PYTHON
    appsettings->setValue(GC_EMBED_PYTHON, embedPython->isChecked());
    if (!embedPython->isChecked()) fixPySettings->disableFixPy();
#endif


    qint32 state=0;

    // general stuff changed ?
#ifdef GC_WANT_HTTP
    if (b4.hyst != hystedit->text().toFloat() ||
        b4.starthttp != startHttp->isChecked())
#else
    if (b4.hyst != hystedit->text().toFloat())
#endif
        state += CONFIG_GENERAL;

    if (b4.wbal != wbalForm->currentIndex())
        state += CONFIG_WBAL;

    // units prefs changed?
    if (b4.unit != unitCombo->currentIndex() ||
        b4.metricRunPace != metricRunPace->isChecked() ||
        b4.metricSwimPace != metricSwimPace->isChecked())
        state += CONFIG_UNITS;

    return state;
}

#ifdef GC_WANT_R
void
GeneralPage::browseRDir()
{
    QString currentDir = rDirectory->text();
    if (!QDir(currentDir).exists()) currentDir = "";
    QString dir = QFileDialog::getExistingDirectory(this, tr("R Installation (R_HOME)"),
                            currentDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir != "") rDirectory->setText(dir);  //only overwrite current dir, if a new was selected
}
#endif
#ifdef GC_WANT_PYTHON
void
GeneralPage::browsePythonDir()
{
    QString currentDir = pythonDirectory->text();
    if (!QDir(currentDir).exists()) currentDir = "";
    QString dir = QFileDialog::getExistingDirectory(this, tr("Python Installation (PYTHONHOME)"),
                            currentDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir != "") {
        QString pybin, pypath;

        // is it installed there?
        bool isgood = PythonEmbed::pythonInstalled(pybin, pypath, dir);

        if (!isgood) {
            QMessageBox nope(QMessageBox::Warning, tr("Invalid Folder"), tr("Python does not appear to be installed in that location.\n"));
            nope.exec();
        } else {
            pythonDirectory->setText(dir);  //only overwrite current dir, if a new was selected
        }
     }
}
#endif

void
GeneralPage::browseWorkoutDir()
{
    QString currentDir = workoutDirectory->text();
    if (!QDir(currentDir).exists()) currentDir = "";
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Workout Library"),
                            currentDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir != "") workoutDirectory->setText(dir);  //only overwrite current dir, if a new was selected
}


void
GeneralPage::browseAthleteDir()
{
    QString currentDir = athleteDirectory->text();
    if (!QDir(currentDir).exists()) currentDir = "";
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Athlete Library"),
                            currentDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir != "") athleteDirectory->setText(dir);  //only overwrite current dir, if a new was selected
}

//
// Realtime devices page
//
DevicePage::DevicePage(QWidget *parent, Context *context) : QWidget(parent), context(context)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    DeviceTypes all;
    devices = all.getList();

    addButton = new QPushButton(tr("+"),this);
    delButton = new QPushButton(tr("-"),this);

    deviceList = new QTableView(this);
#ifdef Q_OS_MAC
    addButton->setText(tr("Add"));
    delButton->setText(tr("Delete"));
    deviceList->setAttribute(Qt::WA_MacShowFocusRect, 0);
#else
    addButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    delButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
#endif
    deviceListModel = new deviceModel(this);

    // replace standard model with ours
    QItemSelectionModel *stdmodel = deviceList->selectionModel();
    deviceList->setModel(deviceListModel);
    delete stdmodel;

    deviceList->setSortingEnabled(false);
    deviceList->setSelectionBehavior(QAbstractItemView::SelectRows);
    deviceList->horizontalHeader()->setStretchLastSection(true);
    deviceList->verticalHeader()->hide();
    deviceList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    deviceList->setSelectionMode(QAbstractItemView::SingleSelection);

    mainLayout->addWidget(deviceList);
    QHBoxLayout *bottom = new QHBoxLayout;
    bottom->setSpacing(2 *dpiXFactor);
    bottom->addStretch();
    bottom->addWidget(addButton);
    bottom->addWidget(delButton);
    mainLayout->addLayout(bottom);

    connect(addButton, SIGNAL(clicked()), this, SLOT(devaddClicked()));
    connect(delButton, SIGNAL(clicked()), this, SLOT(devdelClicked()));
    connect(context, SIGNAL(configChanged(qint32)), deviceListModel, SLOT(doReset()));
}

qint32
DevicePage::saveClicked()
{
    // Save the device configuration...
    DeviceConfigurations all;
    all.writeConfig(deviceListModel->Configuration);

    return 0;
}

void
DevicePage::devaddClicked()
{
    DeviceConfiguration add;
    AddDeviceWizard *p = new AddDeviceWizard(context);
    p->show();
}

void
DevicePage::devdelClicked()
{
    deviceListModel->del();
}

// add a new configuration
void
deviceModel::add(DeviceConfiguration &newone)
{
    insertRows(0,1, QModelIndex());

    // insert name
    QModelIndex index = deviceModel::index(0,0, QModelIndex());
    setData(index, newone.name, Qt::EditRole);

    // insert type
    index = deviceModel::index(0,1, QModelIndex());
    setData(index, newone.type, Qt::EditRole);

    // insert portSpec
    index = deviceModel::index(0,2, QModelIndex());
    setData(index, newone.portSpec, Qt::EditRole);

    // insert Profile
    index = deviceModel::index(0,3, QModelIndex());
    setData(index, newone.deviceProfile, Qt::EditRole);

    // insert virtualPowerIndex
    index = deviceModel::index(0,4, QModelIndex());
    setData(index, newone.postProcess, Qt::EditRole);
}

// delete an existing configuration
void
deviceModel::del()
{
    // which row is selected in the table?
    DevicePage *temp = static_cast<DevicePage*>(parent);
    QItemSelectionModel *selectionModel = temp->deviceList->selectionModel();

    QModelIndexList indexes = selectionModel->selectedRows();
    QModelIndex index;

    foreach (index, indexes) {
        //int row = this->mapToSource(index).row();
        removeRows(index.row(), 1, QModelIndex());
    }
}

deviceModel::deviceModel(QObject *parent) : QAbstractTableModel(parent)
{
    this->parent = parent;

    // get current configuration
    DeviceConfigurations all;
    Configuration = all.getList();
}

void
deviceModel::doReset()
{
    beginResetModel();
    DeviceConfigurations all;
    Configuration = all.getList();
    endResetModel();
}

int
deviceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return Configuration.size();
}

int
deviceModel::columnCount(const QModelIndex &) const
{
    return 5;
}


// setup the headings!
QVariant deviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
     if (role != Qt::DisplayRole) return QVariant(); // no display, no game!

     if (orientation == Qt::Horizontal) {
         switch (section) {
             case 0:
                 return tr("Device Name");
             case 1:
                 return tr("Device Type");
             case 2:
                 return tr("Port Spec");
             case 3:
                 return tr("Profile");
             case 4:
                 return tr("Virtual");
             default:
                 return QVariant();
         }
     }
     return QVariant();
 }

// return data item for row/col specified in index
QVariant deviceModel::data(const QModelIndex &index, int role) const
{
     if (!index.isValid()) return QVariant();
     if (index.row() >= Configuration.size() || index.row() < 0) return QVariant();

     if (role == Qt::DisplayRole) {
         DeviceConfiguration Entry = Configuration.at(index.row());

         switch(index.column()) {
            case 0 : return Entry.name;
                break;
            case 1 :
                {
                DeviceTypes all;
                DeviceType lookupType = all.getType (Entry.type);
                return lookupType.name;
                }
                break;
            case 2 :
                return Entry.portSpec;
                break;
            case 3 :
                return Entry.deviceProfile;
            case 4 :
                return Entry.postProcess;
         }
     }

     // how did we get here!?
     return QVariant();
}

// update the model with new data
bool deviceModel::insertRows(int position, int rows, const QModelIndex &index)
{
     Q_UNUSED(index);
     beginInsertRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; row++) {
         DeviceConfiguration emptyEntry;
         Configuration.insert(position, emptyEntry);
     }
     endInsertRows();
     return true;
}

// delete a row!
bool deviceModel::removeRows(int position, int rows, const QModelIndex &index)
{
     Q_UNUSED(index);
     beginRemoveRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; ++row) {
         Configuration.removeAt(position);
     }

     endRemoveRows();
     return true;
}

bool deviceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
        if (index.isValid() && role == Qt::EditRole) {
            int row = index.row();

            DeviceConfiguration p = Configuration.value(row);

            switch (index.column()) {
                case 0 : //name
                    p.name = value.toString();
                    break;
                case 1 : //type
                    p.type = value.toInt();
                    break;
                case 2 : // spec
                    p.portSpec = value.toString();
                    break;
                case 3 : // Profile
                    p.deviceProfile = value.toString();
                    break;
                case 4 : // Profile
                    p.postProcess = value.toInt();
                    break;
            }
            Configuration.replace(row,p);
                emit(dataChanged(index, index));

            return true;
        }

        return false;
}

//
// Train view options page
//
TrainOptionsPage::TrainOptionsPage(QWidget *parent, Context *context) : QWidget(parent), context(context)
{
    useSimulatedSpeed = new QCheckBox(tr("Use simulated Speed in slope mode"), this);
    useSimulatedSpeed->setChecked(appsettings->value(this, TRAIN_USESIMULATEDSPEED, false).toBool());

    autoConnect = new QCheckBox(tr("Auto-connect devices in Train View"), this);
    autoConnect->setChecked(appsettings->value(this, TRAIN_AUTOCONNECT, false).toBool());

    multiCheck = new QCheckBox(tr("Allow multiple devices in Train View"), this);
    multiCheck->setChecked(appsettings->value(this, TRAIN_MULTI, false).toBool());

    autoHide = new QCheckBox(tr("Auto-hide bottom bar in Train View"), this);
    autoHide->setChecked(appsettings->value(this, TRAIN_AUTOHIDE, false).toBool());

    // Disabled until ported across from the existing bottom bar checkbox
    autoHide->setDisabled(true);

    lapAlert = new QCheckBox(tr("Play sound before new lap"), this);
    lapAlert->setChecked(appsettings->value(this, TRAIN_LAPALERT, false).toBool());

    QVBoxLayout *all = new QVBoxLayout(this);
    all->addWidget(useSimulatedSpeed);
    all->addWidget(multiCheck);
    all->addWidget(autoConnect);
    all->addWidget(autoHide);
    all->addWidget(lapAlert);
    all->addStretch();
}


qint32
TrainOptionsPage::saveClicked()
{
    // Save the train view settings...
    appsettings->setValue(TRAIN_USESIMULATEDSPEED, useSimulatedSpeed->isChecked());
    appsettings->setValue(TRAIN_MULTI, multiCheck->isChecked());
    appsettings->setValue(TRAIN_AUTOCONNECT, autoConnect->isChecked());
    appsettings->setValue(TRAIN_AUTOHIDE, autoHide->isChecked());
    appsettings->setValue(TRAIN_LAPALERT, lapAlert->isChecked());

    return 0;
}


//
// Remote control page
//
RemotePage::RemotePage(QWidget *parent, Context *context) : QWidget(parent), context(context)
{
    remote = new RemoteControl;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    fields = new QTreeWidget;
    fields->headerItem()->setText(0, tr("Action"));
    fields->headerItem()->setText(1, tr("ANT+ Command"));
    fields->setColumnWidth(0,100 *dpiXFactor);
    fields->setColumnWidth(1,200 *dpiXFactor);
    fields->setColumnCount(2);
    fields->setSelectionMode(QAbstractItemView::SingleSelection);
    fields->setIndentation(0);

    fields->setCurrentItem(fields->invisibleRootItem()->child(0));

    mainLayout->addWidget(fields, 0,0);

    // Load the native command list
    QList <RemoteCmd> nativeCmds = remote->getNativeCmds();

    // Load the ant command list
    QList<RemoteCmd> antCmds = remote->getAntCmds();

    // Load the remote control mappings
    QList<CmdMap> cmdMaps = remote->getMappings();

    // create a row for each native command
    int index = 0;
    foreach (RemoteCmd nativeCmd, nativeCmds) {

        QComboBox *comboBox = new QComboBox(this);
        comboBox->addItem("<unset>");

        // populate the combo box with all possible ANT commands
        foreach(RemoteCmd antCmd, antCmds) {
            comboBox->addItem(antCmd.getCmdStr());
        }

        // is this native command mapped to an ANT command?
        foreach(CmdMap cmdMapping, cmdMaps) {
            if (cmdMapping.getNativeCmdId() == nativeCmd.getCmdId()) {
                if (cmdMapping.getAntCmdId() != 0xFFFF) {
                    //qDebug() << "ANT remote mapping found:" << cmdMapping.getNativeCmdId() << cmdMapping.getAntCmdId();

                    int i=0;
                    foreach(RemoteCmd antCmd, antCmds) {
                        i++; // increment first to skip <unset>
                        if (cmdMapping.getAntCmdId() == antCmd.getCmdId()) {
                            // set the default entry for the combo box
                            comboBox->setCurrentIndex(i);
                        }
                    }
                }
            }
        }

        QTreeWidgetItem *add = new QTreeWidgetItem;
        fields->invisibleRootItem()->insertChild(index, add);
        add->setFlags(add->flags() | Qt::ItemIsEditable);

        add->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
        add->setText(0, nativeCmd.getDisplayStr());

        add->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);
        fields->setItemWidget(add, 1, comboBox);
        index++;
    }
}

qint32
RemotePage::saveClicked()
{
    // Save the remote control code mappings...

    //qDebug() << "RemotePage::saveClicked()";

    // Load the ant command list
    QList<RemoteCmd> antCmds = remote->getAntCmds();

    // Load the remote control mappings
    QList<CmdMap> cmdMaps = remote->getMappings();

    for(int i=0; i < cmdMaps.size(); i++) {

        QWidget *button = fields->itemWidget(fields->invisibleRootItem()->child(i),1);
        int index = ((QComboBox*)button)->currentIndex();

        if (index) {
            cmdMaps[i].setAntCmdId(antCmds[index-1].getCmdId());

        } else {
            cmdMaps[i].setAntCmdId(0xFFFF); // no command
        }
    }

    remote->writeConfig(cmdMaps);
    return 0;
}

const SimBicyclePartEntry& SimBicyclePage::GetSimBicyclePartEntry(int e)
{
    // Bike mass values approximate a good current bike. Wheels are shimano c40 with conti tubeless tires.

    static const SimBicyclePartEntry arr[] = {
          // SpinBox Title                              Path to athlete value                Default Value      Decimal      Tooltip                                                                              enum
        { tr("Bicycle Mass Without Wheels (g)"     )  , GC_SIM_BICYCLE_MASSWITHOUTWHEELSG,   4000,              0,           tr("Mass of everything that isn't wheels, tires, skewers...")},                       // BicycleWithoutWheelsG
        { tr("Front Wheel Mass (g)"                )  , GC_SIM_BICYCLE_FRONTWHEELG,          739,               0,           tr("Mass of front wheel including tires and skewers...")},                            // FrontWheelG
        { tr("Front Spoke Count"                   )  , GC_SIM_BICYCLE_FRONTSPOKECOUNT,      24,                0,           tr("")},                                                                              // FrontSpokeCount
        { tr("Front Spoke & Nipple Mass - Each (g)")  , GC_SIM_BICYCLE_FRONTSPOKENIPPLEG,    5.6,               1,           tr("Mass of a single spoke and nipple, washers, etc.")},                              // FrontSpokeNippleG
        { tr("Front Rim Mass (g)"                  )  , GC_SIM_BICYCLE_FRONTRIMG,            330,               0,           tr("")},                                                                              // FrontRimG
        { tr("Front Rotor Mass (g)"                )  , GC_SIM_BICYCLE_FRONTROTORG,          120,               0,           tr("Mass of rotor including bolts")},                                                 // FrontRotorG
        { tr("Front Skewer Mass (g)"               )  , GC_SIM_BICYCLE_FRONTSKEWERG,         40,                0,           tr("")},                                                                              // FrontSkewerG
        { tr("Front Tire Mass (g)"                 )  , GC_SIM_BICYCLE_FRONTTIREG,           220,               0,           tr("")},                                                                              // FrontTireG
        { tr("Front Tube or Sealant Mass (g)"      )  , GC_SIM_BICYCLE_FRONTTUBESEALANTG,    26,                0,           tr("Mass of anything inside the tire: sealant, tube...")},                            // FrontTubeSealantG
        { tr("Front Rim Outer Radius (m)"          )  , GC_SIM_BICYCLE_FRONTOUTERRADIUSM,    .35,               3,           tr("Functional outer radius of wheel, used for computing wheel circumference")},      // FrontOuterRadiusM
        { tr("Front Rim Inner Radius (m)"          )  , GC_SIM_BICYCLE_FRONTRIMINNERRADIUSM, .3,                3,           tr("Inner radius of rim, for computing wheel inertia")},                              // FrontRimInnerRadiusM
        { tr("Rear Wheel Mass (g)"                 )  , GC_SIM_BICYCLE_REARWHEELG,           739,               0,           tr("Mass of front wheel including tires and skewers...")},                            // RearWheelG
        { tr("Rear Spoke Count"                    )  , GC_SIM_BICYCLE_REARSPOKECOUNT,       24,                0,           tr("")},                                                                              // RearSpokeCount
        { tr("Rear Spoke & Nipple Mass - Each (g)" )  , GC_SIM_BICYCLE_REARSPOKENIPPLEG,     5.6,               1,           tr("Mass of a single spoke and nipple, washers, etc.")},                              // RearSpokeNippleG
        { tr("Rear Rim Mass (g)"                   )  , GC_SIM_BICYCLE_REARRIMG,             330,               0,           tr("")},                                                                              // RearRimG
        { tr("Rear Rotor Mass (g)"                 )  , GC_SIM_BICYCLE_REARROTORG,           120,               0,           tr("Mass of rotor including bolts")},                                                 // RearRotorG
        { tr("Rear Skewer Mass (g)"                )  , GC_SIM_BICYCLE_REARSKEWERG,           40,               0,           tr("Mass of skewer/axle/funbolts, etc...")},                                          // RearSkewerG
        { tr("Rear Tire Mass (g)"                  )  , GC_SIM_BICYCLE_REARTIREG,            220,               0,           tr("Mass of tire not including tube or sealant")},                                    // RearTireG
        { tr("Rear Tube or Sealant Mass (g)"       )  , GC_SIM_BICYCLE_REARTUBESEALANTG,      26,               0,           tr("Mass of anything inside the tire: sealant, tube...")},                            // RearTubeSealantG
        { tr("Rear Rim Outer Radius (m)"           )  , GC_SIM_BICYCLE_REAROUTERRADIUSM,     .35,               3,           tr("Functional outer radius of wheel, used for computing wheel circumference")},      // RearOuterRadiusM
        { tr("Rear Rim Inner Radius (m)"           )  , GC_SIM_BICYCLE_REARRIMINNERRADIUSM,  .3,                3,           tr("Inner radius of rim, for computing wheel inertia")},                              // RearRimInnerRadiusM
        { tr("Rear Cassette Mass(g)"               )  , GC_SIM_BICYCLE_CASSETTEG,            190,               0,           tr("Mass of rear cassette, including lockring")},                                     // CassetteG
        { tr("Coefficient of rolling resistance"   )  , GC_SIM_BICYCLE_CRR,                  0.004,             4,           tr("Total coefficient of rolling resistance for bicycle")},                           // CRR
        { tr("Coefficient of power train loss"     )  , GC_SIM_BICYCLE_Cm,                   1.0,               3,           tr("Power train loss between reported watts and wheel. For direct drive trainer like kickr there is no relevant loss and value shold be 1.0.")},      // Cm
        { tr("Coefficient of drag"                 )  , GC_SIM_BICYCLE_Cd,        (1.0 - 0.0045),               5,           tr("Coefficient of drag of rider and bicycle")},                                      // Cd
        { tr("Frontal Area (m^2)"                  )  , GC_SIM_BICYCLE_Am2,                  0.5,               2,           tr("Effective frontal area of rider and bicycle")},                                   // Am2
        { tr("Temperature (K)"                     )  , GC_SIM_BICYCLE_Tk,                 293.15,              2,           tr("Temperature in kelvin, used with altitude to compute air density")}               // Tk
    };

    if (e < 0 || e >= LastPart) e = 0;

    return arr[e];
}

double
SimBicyclePage::GetBicyclePartValue(Context* context, int e)
{
    const SimBicyclePartEntry &r = GetSimBicyclePartEntry(e);

    if (!context) return r.m_defaultValue;

    return appsettings->cvalue(
        context->athlete->cyclist,
        r.m_path,
        r.m_defaultValue).toDouble();
}

void
SimBicyclePage::AddSpecBox(int ePart)
{
    const SimBicyclePartEntry & entry = GetSimBicyclePartEntry(ePart);

    m_LabelArr[ePart] = new QLabel(entry.m_label);

    QDoubleSpinBox * pSpinBox = new QDoubleSpinBox(this);

    pSpinBox->setMaximum(99999);
    pSpinBox->setMinimum(0.0);
    pSpinBox->setDecimals(entry.m_decimalPlaces);
    pSpinBox->setValue(GetBicyclePartValue(context, ePart));
    pSpinBox->setToolTip(entry.m_tooltip);
    double singlestep = 1.;
    for (int i = 0; i < entry.m_decimalPlaces; i++)
        singlestep /= 10.;

    pSpinBox->setSingleStep(singlestep);

    m_SpinBoxArr[ePart] = pSpinBox;
}

void
SimBicyclePage::SetStatsLabelArray(double )
{
    double riderMassKG = 0;

    const double bicycleMassWithoutWheelsG = m_SpinBoxArr[SimBicyclePage::BicycleWithoutWheelsG]->value();
    const double bareFrontWheelG           = m_SpinBoxArr[SimBicyclePage::FrontWheelG          ]->value();
    const double frontSpokeCount           = m_SpinBoxArr[SimBicyclePage::FrontSpokeCount      ]->value();
    const double frontSpokeNippleG         = m_SpinBoxArr[SimBicyclePage::FrontSpokeNippleG    ]->value();
    const double frontWheelOuterRadiusM    = m_SpinBoxArr[SimBicyclePage::FrontOuterRadiusM    ]->value();
    const double frontRimInnerRadiusM      = m_SpinBoxArr[SimBicyclePage::FrontRimInnerRadiusM ]->value();
    const double frontRimG                 = m_SpinBoxArr[SimBicyclePage::FrontRimG            ]->value();
    const double frontRotorG               = m_SpinBoxArr[SimBicyclePage::FrontRotorG          ]->value();
    const double frontSkewerG              = m_SpinBoxArr[SimBicyclePage::FrontSkewerG         ]->value();
    const double frontTireG                = m_SpinBoxArr[SimBicyclePage::FrontTireG           ]->value();
    const double frontTubeOrSealantG       = m_SpinBoxArr[SimBicyclePage::FrontTubeSealantG    ]->value();
    const double bareRearWheelG            = m_SpinBoxArr[SimBicyclePage::RearWheelG           ]->value();
    const double rearSpokeCount            = m_SpinBoxArr[SimBicyclePage::RearSpokeCount       ]->value();
    const double rearSpokeNippleG          = m_SpinBoxArr[SimBicyclePage::RearSpokeNippleG     ]->value();
    const double rearWheelOuterRadiusM     = m_SpinBoxArr[SimBicyclePage::RearOuterRadiusM     ]->value();
    const double rearRimInnerRadiusM       = m_SpinBoxArr[SimBicyclePage::RearRimInnerRadiusM  ]->value();
    const double rearRimG                  = m_SpinBoxArr[SimBicyclePage::RearRimG             ]->value();
    const double rearRotorG                = m_SpinBoxArr[SimBicyclePage::RearRotorG           ]->value();
    const double rearSkewerG               = m_SpinBoxArr[SimBicyclePage::RearSkewerG          ]->value();
    const double rearTireG                 = m_SpinBoxArr[SimBicyclePage::RearTireG            ]->value();
    const double rearTubeOrSealantG        = m_SpinBoxArr[SimBicyclePage::RearTubeSealantG     ]->value();
    const double cassetteG                 = m_SpinBoxArr[SimBicyclePage::CassetteG            ]->value();

    const double frontWheelG = bareFrontWheelG + frontRotorG + frontSkewerG + frontTireG + frontTubeOrSealantG;
    const double frontWheelRotatingG = frontRimG + frontTireG + frontTubeOrSealantG + (frontSpokeCount * frontSpokeNippleG);
    const double frontWheelCenterG = frontWheelG - frontWheelRotatingG;

    BicycleWheel frontWheel(frontWheelOuterRadiusM, frontRimInnerRadiusM, frontWheelG / 1000, frontWheelCenterG /1000, frontSpokeCount, frontSpokeNippleG/1000);

    const double rearWheelG = bareRearWheelG + cassetteG + rearRotorG + rearSkewerG + rearTireG + rearTubeOrSealantG;
    const double rearWheelRotatingG = rearRimG + rearTireG + rearTubeOrSealantG + (rearSpokeCount * rearSpokeNippleG);
    const double rearWheelCenterG = rearWheelG - rearWheelRotatingG;

    BicycleWheel rearWheel (rearWheelOuterRadiusM,  rearRimInnerRadiusM,  rearWheelG / 1000,  rearWheelCenterG / 1000,  rearSpokeCount,  rearSpokeNippleG/1000);

    BicycleConstants constants(
        m_SpinBoxArr[SimBicyclePage::CRR]->value(),
        m_SpinBoxArr[SimBicyclePage::Cm] ->value(),
        m_SpinBoxArr[SimBicyclePage::Cd] ->value(),
        m_SpinBoxArr[SimBicyclePage::Am2]->value(),
        m_SpinBoxArr[SimBicyclePage::Tk] ->value());

    Bicycle bicycle(NULL, constants, riderMassKG, bicycleMassWithoutWheelsG / 1000., frontWheel, rearWheel);

    m_StatsLabelArr[StatsLabel]              ->setText(QString(tr("------ Derived Stats -------")));
    m_StatsLabelArr[StatsTotalKEMass]        ->setText(QString(tr("Total KEMass:         \t%1g")).arg(bicycle.KEMass()));
    m_StatsLabelArr[StatsFrontWheelKEMass]   ->setText(QString(tr("FrontWheel KEMass:    \t%1g")).arg(bicycle.FrontWheel().KEMass() * 1000));
    m_StatsLabelArr[StatsFrontWheelMass]     ->setText(QString(tr("FrontWheel Mass:      \t%1g")).arg(bicycle.FrontWheel().MassKG() * 1000));
    m_StatsLabelArr[StatsFrontWheelEquivMass]->setText(QString(tr("FrontWheel EquivMass: \t%1g")).arg(bicycle.FrontWheel().EquivalentMassKG() * 1000));
    m_StatsLabelArr[StatsFrontWheelI]        ->setText(QString(tr("FrontWheel I:         \t%1")).arg(bicycle.FrontWheel().I()));
    m_StatsLabelArr[StatsRearWheelKEMass]    ->setText(QString(tr("Rear Wheel KEMass:    \t%1g")).arg(bicycle.RearWheel().KEMass() * 1000));
    m_StatsLabelArr[StatsRearWheelMass]      ->setText(QString(tr("Rear Wheel Mass:      \t%1g")).arg(bicycle.RearWheel().MassKG() * 1000));
    m_StatsLabelArr[StatsRearWheelEquivMass] ->setText(QString(tr("Rear Wheel EquivMass: \t%1g")).arg(bicycle.RearWheel().EquivalentMassKG() * 1000));
    m_StatsLabelArr[StatsRearWheelI]         ->setText(QString(tr("Rear Wheel I:         \t%1")).arg(bicycle.RearWheel().I()));
}


SimBicyclePage::SimBicyclePage(QWidget *parent, Context *context) : QWidget(parent), context(context)
{
    QVBoxLayout *all = new QVBoxLayout(this);
    QGridLayout *grid = new QGridLayout;

#ifdef Q_OS_MAX
    setContentsMargins(10, 10, 10, 10);
    grid->setSpacing(5 * dpiXFactor);
    all->setSpacing(5 * dpiXFactor);
#endif

    // Populate m_LabelArr and m_SpinBoxArr
    for (int e = 0; e < LastPart; e++)
    {
        AddSpecBox(e);
    }

    Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter;

    // Two sections. Bike mass properties are in two rows to the left.
    // Other properties like cd, ca and temp go in section to the right.

    int Section1Start = 0;
    int Section1End = BicycleParts::CRR;
    int Section2Start = Section1End;
    int Section2End = BicycleParts::LastPart;

    // Column 0
    int column = 0;
    int row = 0;
    for (int i = Section1Start; i < Section1End; i++) {
        grid->addWidget(m_LabelArr[i], row, column, alignment);
        row++;
    }

    // Column 1
    column = 1;
    row = 0;
    for (int i = Section1Start; i < Section1End; i++) {
        grid->addWidget(m_SpinBoxArr[i], row, column, alignment);
        row++;
    }

    // Column 2
    column = 2;
    row = 0;
    grid->addWidget(new QLabel("These values are used to compute correct inertia\n"
                               "for simulated speed in trainer mode.These values\n"
                               "only have effect when the 'Use simulated speed in\n"
                               "slope mode' option is set on the training preferences\n"
                               " tab."), row, column, alignment);

    int section2FirstRow = row + 1;

    // Now add section 2.
    row = section2FirstRow;
    for (int i = Section2Start; i < Section2End; i++) {
        grid->addWidget(m_LabelArr[i], row, column, alignment);
        row++;
    }

    column++;

    row = section2FirstRow;
    for (int i = Section2Start; i < Section2End; i++) {
        grid->addWidget(m_SpinBoxArr[i], row, column, alignment);
        row++;
    }

    // There is still room below section 2... lets put in some useful stats
    // about the virtual bicycle.

    int statsFirstRow = row + 1;
    column = 2;

    // Create Stats Labels
    for (int i = StatsLabel; i < StatsLastPart; i++) {
        m_StatsLabelArr[i] = new QLabel();
    }

    // Populate Stats Labels
    SetStatsLabelArray();

    row = statsFirstRow;
    for (int i = StatsLabel; i < StatsLastPart; i++) {
        grid->addWidget(m_StatsLabelArr[i], row, column, alignment);
        row++;
    }

    all->addLayout(grid);
    all->addStretch();

    for (int i = 0; i < LastPart; i++) {
        connect(m_SpinBoxArr[i], SIGNAL(valueChanged(double)), this, SLOT(SetStatsLabelArray(double)));
    }

}

qint32
SimBicyclePage::saveClicked()
{
    for (int e = 0; e < BicycleParts::LastPart; e++) {
        const SimBicyclePartEntry& entry = GetSimBicyclePartEntry(e);
        appsettings->setCValue(context->athlete->cyclist, entry.m_path, m_SpinBoxArr[e]->value());
    }

    qint32 state = CONFIG_ATHLETE;

    return state;
}


static double scalefactors[9] = { 0.5f, 0.6f, 0.8, 0.9, 1.0f, 1.1f, 1.25f, 1.5f, 2.0f };

//
// Appearances page
//
ColorsPage::ColorsPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    themes = new QTreeWidget;
    themes->headerItem()->setText(0, tr("Swatch"));
    themes->headerItem()->setText(1, tr("Name"));
    themes->setColumnCount(2);
    themes->setColumnWidth(0,240 *dpiXFactor);
    themes->setSelectionMode(QAbstractItemView::SingleSelection);
    //colors->setEditTriggers(QAbstractItemView::SelectedClicked); // allow edit
    themes->setUniformRowHeights(true); // causes height problems when adding - in case of non-text fields
    themes->setIndentation(0);
    //colors->header()->resizeSection(0,300);

    colors = new QTreeWidget;
    colors->headerItem()->setText(0, tr("Color"));
    colors->headerItem()->setText(1, tr("Select"));
    colors->setColumnCount(2);
    colors->setColumnWidth(0,350 *dpiXFactor);
    colors->setSelectionMode(QAbstractItemView::NoSelection);
    //colors->setEditTriggers(QAbstractItemView::SelectedClicked); // allow edit
    colors->setUniformRowHeights(true); // causes height problems when adding - in case of non-text fields
    colors->setIndentation(0);
    //colors->header()->resizeSection(0,300);

    QLabel *antiAliasLabel = new QLabel(tr("Antialias"));
    antiAliased = new QCheckBox;
    antiAliased->setChecked(appsettings->value(this, GC_ANTIALIAS, true).toBool());
#ifndef Q_OS_MAC
    QLabel *rideScrollLabel = new QLabel(tr("Activity Scrollbar"));
    rideScroll = new QCheckBox;
    rideScroll->setChecked(appsettings->value(this, GC_RIDESCROLL, true).toBool());
    QLabel *rideHeadLabel = new QLabel(tr("Activity Headings"));
    rideHead = new QCheckBox;
    rideHead->setChecked(appsettings->value(this, GC_RIDEHEAD, true).toBool());
#endif
    lineWidth = new QDoubleSpinBox;
    lineWidth->setMaximum(5);
    lineWidth->setMinimum(0.5);
    lineWidth->setSingleStep(0.5);
    applyTheme = new QPushButton(tr("Apply Theme"));
    lineWidth->setValue(appsettings->value(this, GC_LINEWIDTH, 0.5).toDouble());

    QLabel *lineWidthLabel = new QLabel(tr("Line Width"));
    QLabel *defaultLabel = new QLabel(tr("Font"));
    QLabel *scaleLabel = new QLabel(tr("Font Scaling" ));

    def = new QFontComboBox(this);

    // get round QTBUG
    def->setCurrentIndex(0);
    def->setCurrentIndex(1);
    def->setCurrentFont(baseFont);
    def->setCurrentFont(baseFont);

    // font scaling
    double scale = appsettings->value(this, GC_FONT_SCALE, 1.0).toDouble();
    fontscale = new QSlider(this);
    fontscale->setMinimum(0);
    fontscale->setMaximum(8);
    fontscale->setTickInterval(1);
    fontscale->setValue(3);
    fontscale->setOrientation(Qt::Horizontal);
    for(int i=0; i<7; i++) {
        if (scalefactors[i] == scale) {
            fontscale->setValue(i);
            break;
        }
    }

    QFont font=baseFont;
    font.setPointSizeF(baseFont.pointSizeF() * scale);
    fonttext = new QLabel(this);
    fonttext->setText("The quick brown fox jumped over the lazy dog");
    fonttext->setFont(font);
    fonttext->setFixedHeight(30 * dpiYFactor);
    fonttext->setFixedWidth(330 * dpiXFactor);

    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(5 *dpiXFactor);

    grid->addWidget(defaultLabel, 0,0);
    grid->addWidget(scaleLabel, 1,0);

    grid->addWidget(lineWidthLabel, 0,3);
    grid->addWidget(lineWidth, 0,4);
    grid->addWidget(antiAliasLabel, 1,3);
    grid->addWidget(antiAliased, 1,4);
#ifndef Q_OS_MAC
    grid->addWidget(rideScrollLabel, 2,3);
    grid->addWidget(rideScroll, 2,4);
    grid->addWidget(rideHeadLabel, 3,3);
    grid->addWidget(rideHead, 3,4);
#endif

    grid->addWidget(def, 0,1, Qt::AlignVCenter|Qt::AlignLeft);
    grid->addWidget(fontscale, 1,1);
    grid->addWidget(fonttext, 2,0,1,2);

    grid->setColumnStretch(0,1);
    grid->setColumnStretch(1,4);
    grid->setColumnStretch(2,1);
    grid->setColumnStretch(3,1);
    grid->setColumnStretch(4,4);

    mainLayout->addLayout(grid);

    colorTab = new QTabWidget(this);
    colorTab->addTab(themes, tr("Theme"));
    colorTab->addTab(colors, tr("Colors"));
    colorTab->setCornerWidget(applyTheme);

    mainLayout->addWidget(colorTab);

    colorSet = GCColor::colorSet();
    for (int i=0; colorSet[i].name != ""; i++) {

        QTreeWidgetItem *add;
        ColorButton *colorButton = new ColorButton(this, colorSet[i].name, colorSet[i].color);
        add = new QTreeWidgetItem(colors->invisibleRootItem());
        add->setText(0, colorSet[i].name);
        colors->setItemWidget(add, 1, colorButton);

    }
    connect(applyTheme, SIGNAL(clicked()), this, SLOT(applyThemeClicked()));

    foreach(ColorTheme theme, GCColor::themes().themes) {

        QTreeWidgetItem *add;
        ColorLabel *swatch = new ColorLabel(theme);
        swatch->setFixedHeight(30*dpiYFactor);
        add = new QTreeWidgetItem(themes->invisibleRootItem());
        themes->setItemWidget(add, 0, swatch);
        add->setText(1, theme.name);

    }
    connect(colorTab, SIGNAL(currentChanged(int)), this, SLOT(tabChanged()));
    connect(def, SIGNAL(currentFontChanged(QFont)), this, SLOT(scaleFont()));
    connect(fontscale, SIGNAL(valueChanged(int)), this, SLOT(scaleFont()));

    // save initial values
    b4.alias = antiAliased->isChecked();
#ifndef Q_OS_MAC
    b4.scroll = rideScroll->isChecked();
    b4.head = rideHead->isChecked();
#endif
    b4.line = lineWidth->value();
    b4.fingerprint = Colors::fingerprint(colorSet);
}

void
ColorsPage::scaleFont()
{
    QFont font=baseFont;
    font.setFamily(def->currentFont().family());
    font.setPointSizeF(baseFont.pointSizeF()*scalefactors[fontscale->value()]);
    fonttext->setFont(font);
}

void
ColorsPage::tabChanged()
{
    // are we on the them page
    if (colorTab->currentIndex() == 0) applyTheme->show();
    else applyTheme->hide();
}

void
ColorsPage::applyThemeClicked()
{
    int index=0;

    // first check we have a selection!
    if (themes->currentItem() && (index=themes->invisibleRootItem()->indexOfChild(themes->currentItem())) >= 0) {

        // now get the theme selected
        ColorTheme theme = GCColor::themes().themes[index];

        // reset to base
        colorSet = GCColor::defaultColorSet();

        // reset the color selection tools
        colors->clear();
        for (int i=0; colorSet[i].name != ""; i++) {

            QColor color;

            // apply theme to color
            switch(i) {

            case CPLOTBACKGROUND:
            case CRIDEPLOTBACKGROUND:
            case CTRENDPLOTBACKGROUND:
            case CTRAINPLOTBACKGROUND:
                color = theme.colors[0]; // background color
                break;

            case COVERVIEWBACKGROUND:
                // set back to light black for dark themes
                // and gray for light themes
                color = theme.colors[10];
                break;

            case CCARDBACKGROUND:
                // set back to light black for dark themes
                // and gray for light themes
                color = theme.colors[11];
                break;

            case CCHROME:
                //  set to black for dark themese and grey for light themes
                color = theme.colors[1];
                break;

            case CPLOTSYMBOL:
            case CRIDEPLOTXAXIS:
            case CRIDEPLOTYAXIS:
            case CPLOTMARKER:
                color = theme.colors[2]; // accent color
                break;

            case CPLOTSELECT:
            case CPLOTTRACKER:
            case CINTERVALHIGHLIGHTER:
                color = theme.colors[3]; // select color
                break;


            case CPLOTGRID: // grid doesn't have a theme color
                            // we make it barely distinguishable from background
                {
                    QColor bg = theme.colors[0];
                    if(bg == QColor(Qt::black)) color = QColor(30,30,30);
                    else color = bg.darker(110);
                }
                break;

            case CCP:
            case CWBAL:
            case CRIDECP:
                color = theme.colors[4];
                break;

            case CHEARTRATE:
                color = theme.colors[5];
                break;

            case CSPEED:
                color = theme.colors[6];
                break;

            case CPOWER:
                color = theme.colors[7];
                break;

            case CCADENCE:
                color = theme.colors[8];
                break;

            case CTORQUE:
                color = theme.colors[9];
                break;

                default:
                    color = colorSet[i].color;
            }

            QTreeWidgetItem *add;
            ColorButton *colorButton = new ColorButton(this, colorSet[i].name, color);
            add = new QTreeWidgetItem(colors->invisibleRootItem());
            add->setText(0, colorSet[i].name);
            colors->setItemWidget(add, 1, colorButton);

        }
    }
}

qint32
ColorsPage::saveClicked()
{
    appsettings->setValue(GC_LINEWIDTH, lineWidth->value());
    appsettings->setValue(GC_ANTIALIAS, antiAliased->isChecked());
    appsettings->setValue(GC_FONT_SCALE, scalefactors[fontscale->value()]);
    appsettings->setValue(GC_FONT_DEFAULT, def->font().family());
#ifndef Q_OS_MAC
    appsettings->setValue(GC_RIDESCROLL, rideScroll->isChecked());
    appsettings->setValue(GC_RIDEHEAD, rideHead->isChecked());
#endif

    // run down and get the current colors and save
    for (int i=0; colorSet[i].name != ""; i++) {
        QTreeWidgetItem *current = colors->invisibleRootItem()->child(i);
        QColor newColor = ((ColorButton*)colors->itemWidget(current, 1))->getColor();
        QString colorstring = QString("%1:%2:%3").arg(newColor.red())
                                                 .arg(newColor.green())
                                                 .arg(newColor.blue());
        appsettings->setValue(colorSet[i].setting, colorstring);
    }

    // update basefont family
    baseFont.setFamily(def->currentFont().family());

    // application font needs to adapt to scale
    QFont font = baseFont;
    font.setPointSizeF(baseFont.pointSizeF() * scalefactors[fontscale->value()]);
    QApplication::setFont(font);

    // set the "old" user settings to ensure charts adopt etc
    appsettings->setValue(GC_FONT_DEFAULT, def->currentFont().toString());
    appsettings->setValue(GC_FONT_CHARTLABELS, def->currentFont().toString());
    appsettings->setValue(GC_FONT_DEFAULT_SIZE, font.pointSizeF());
    appsettings->setValue(GC_FONT_CHARTLABELS_SIZE, font.pointSizeF() * 0.8);

    // reread into colorset so we can check for changes
    GCColor::readConfig();

    // did we change anything ?
    if(b4.alias != antiAliased->isChecked() ||
#ifndef Q_OS_MAC // not needed on mac
       b4.scroll != rideScroll->isChecked() ||
       b4.head != rideHead->isChecked() ||
#endif
       b4.line != lineWidth->value() ||
       b4.fontscale != scalefactors[fontscale->value()] ||
       b4.fingerprint != Colors::fingerprint(colorSet))
        return CONFIG_APPEARANCE;
    else
        return 0;
}

IntervalMetricsPage::IntervalMetricsPage(QWidget *parent) :
    QWidget(parent), changed(false)
{
    HelpWhatsThis *help = new HelpWhatsThis(this);
    this->setWhatsThis(help->getWhatsThisText(HelpWhatsThis::Preferences_Metrics_Intervals));

    availList = new QListWidget;
    availList->setSortingEnabled(true);
    availList->setSelectionMode(QAbstractItemView::SingleSelection);
    QVBoxLayout *availLayout = new QVBoxLayout;
    availLayout->addWidget(new QLabel(tr("Available Metrics")));
    availLayout->addWidget(availList);
    selectedList = new QListWidget;
    selectedList->setSelectionMode(QAbstractItemView::SingleSelection);
    QVBoxLayout *selectedLayout = new QVBoxLayout;
    selectedLayout->addWidget(new QLabel(tr("Selected Metrics")));
    selectedLayout->addWidget(selectedList);
#ifndef Q_OS_MAC
    upButton = new QToolButton(this);
    downButton = new QToolButton(this);
    leftButton = new QToolButton(this);
    rightButton = new QToolButton(this);
    upButton->setArrowType(Qt::UpArrow);
    downButton->setArrowType(Qt::DownArrow);
    leftButton->setArrowType(Qt::LeftArrow);
    rightButton->setArrowType(Qt::RightArrow);
    upButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    downButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    leftButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    rightButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
#else
    upButton = new QPushButton(tr("Up"));
    downButton = new QPushButton(tr("Down"));
    leftButton = new QPushButton("<");
    rightButton = new QPushButton(">");
#endif
    QVBoxLayout *buttonGrid = new QVBoxLayout;
    QHBoxLayout *upLayout = new QHBoxLayout;
    QHBoxLayout *inexcLayout = new QHBoxLayout;
    QHBoxLayout *downLayout = new QHBoxLayout;

    upLayout->addStretch();
    upLayout->addWidget(upButton);
    upLayout->addStretch();

    inexcLayout->addStretch();
    inexcLayout->addWidget(leftButton);
    inexcLayout->addWidget(rightButton);
    inexcLayout->addStretch();

    downLayout->addStretch();
    downLayout->addWidget(downButton);
    downLayout->addStretch();

    buttonGrid->addStretch();
    buttonGrid->addLayout(upLayout);
    buttonGrid->addLayout(inexcLayout);
    buttonGrid->addLayout(downLayout);
    buttonGrid->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addLayout(availLayout);
    hlayout->addLayout(buttonGrid);
    hlayout->addLayout(selectedLayout);
    setLayout(hlayout);

    QString s;
    if (appsettings->contains(GC_SETTINGS_INTERVAL_METRICS))
        s = appsettings->value(this, GC_SETTINGS_INTERVAL_METRICS).toString();
    else
        s = GC_SETTINGS_INTERVAL_METRICS_DEFAULT;
    QStringList selectedMetrics = s.split(",");

    const RideMetricFactory &factory = RideMetricFactory::instance();
    for (int i = 0; i < factory.metricCount(); ++i) {
        QString symbol = factory.metricName(i);
        if (selectedMetrics.contains(symbol) || symbol.startsWith("compatibility_"))
            continue;
        QSharedPointer<RideMetric> m(factory.newMetric(symbol));
        QListWidgetItem *item = new QListWidgetItem(Utils::unprotect(m->name()));
        item->setData(Qt::UserRole, symbol);
        item->setToolTip(m->description());
        availList->addItem(item);
    }
    foreach (QString symbol, selectedMetrics) {
        if (!factory.haveMetric(symbol))
            continue;
        QSharedPointer<RideMetric> m(factory.newMetric(symbol));
        QListWidgetItem *item = new QListWidgetItem(Utils::unprotect(m->name()));
        item->setData(Qt::UserRole, symbol);
        item->setToolTip(m->description());
        selectedList->addItem(item);
    }

    upButton->setEnabled(false);
    downButton->setEnabled(false);
    leftButton->setEnabled(false);
    rightButton->setEnabled(false);

    connect(upButton, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(downClicked()));
    connect(leftButton, SIGNAL(clicked()), this, SLOT(leftClicked()));
    connect(rightButton, SIGNAL(clicked()), this, SLOT(rightClicked()));
    connect(availList, SIGNAL(itemSelectionChanged()),
            this, SLOT(availChanged()));
    connect(selectedList, SIGNAL(itemSelectionChanged()),
            this, SLOT(selectedChanged()));
}

void
IntervalMetricsPage::upClicked()
{
    assert(!selectedList->selectedItems().isEmpty());
    QListWidgetItem *item = selectedList->selectedItems().first();
    int row = selectedList->row(item);
    assert(row > 0);
    selectedList->takeItem(row);
    selectedList->insertItem(row - 1, item);
    selectedList->setCurrentItem(item);
    changed = true;
}

void
IntervalMetricsPage::downClicked()
{
    assert(!selectedList->selectedItems().isEmpty());
    QListWidgetItem *item = selectedList->selectedItems().first();
    int row = selectedList->row(item);
    assert(row < selectedList->count() - 1);
    selectedList->takeItem(row);
    selectedList->insertItem(row + 1, item);
    selectedList->setCurrentItem(item);
    changed = true;
}

void
IntervalMetricsPage::leftClicked()
{
    assert(!selectedList->selectedItems().isEmpty());
    QListWidgetItem *item = selectedList->selectedItems().first();
    selectedList->takeItem(selectedList->row(item));
    availList->addItem(item);
    changed = true;
    selectedChanged();
}

void
IntervalMetricsPage::rightClicked()
{
    assert(!availList->selectedItems().isEmpty());
    QListWidgetItem *item = availList->selectedItems().first();
    availList->takeItem(availList->row(item));
    selectedList->addItem(item);
    changed = true;
}

void
IntervalMetricsPage::availChanged()
{
    rightButton->setEnabled(!availList->selectedItems().isEmpty());
}

void
IntervalMetricsPage::selectedChanged()
{
    if (selectedList->selectedItems().isEmpty()) {
        upButton->setEnabled(false);
        downButton->setEnabled(false);
        leftButton->setEnabled(false);
        return;
    }
    QListWidgetItem *item = selectedList->selectedItems().first();
    int row = selectedList->row(item);
    if (row == 0)
        upButton->setEnabled(false);
    else
        upButton->setEnabled(true);
    if (row == selectedList->count() - 1)
        downButton->setEnabled(false);
    else
        downButton->setEnabled(true);
    leftButton->setEnabled(true);
}

qint32
IntervalMetricsPage::saveClicked()
{
    if (!changed) return 0;

    QStringList metrics;
    for (int i = 0; i < selectedList->count(); ++i)
        metrics << selectedList->item(i)->data(Qt::UserRole).toString();
    appsettings->setValue(GC_SETTINGS_INTERVAL_METRICS, metrics.join(","));

    return 0;
}

BestsMetricsPage::BestsMetricsPage(QWidget *parent) :
    QWidget(parent), changed(false)
{
    HelpWhatsThis *help = new HelpWhatsThis(this);
    this->setWhatsThis(help->getWhatsThisText(HelpWhatsThis::Preferences_Metrics_Best));

    availList = new QListWidget;
    availList->setSortingEnabled(true);
    availList->setSelectionMode(QAbstractItemView::SingleSelection);
    QVBoxLayout *availLayout = new QVBoxLayout;
    availLayout->addWidget(new QLabel(tr("Available Metrics")));
    availLayout->addWidget(availList);
    selectedList = new QListWidget;
    selectedList->setSelectionMode(QAbstractItemView::SingleSelection);
    QVBoxLayout *selectedLayout = new QVBoxLayout;
    selectedLayout->addWidget(new QLabel(tr("Selected Metrics")));
    selectedLayout->addWidget(selectedList);
#ifndef Q_OS_MAC
    upButton = new QToolButton(this);
    downButton = new QToolButton(this);
    leftButton = new QToolButton(this);
    rightButton = new QToolButton(this);
    upButton->setArrowType(Qt::UpArrow);
    downButton->setArrowType(Qt::DownArrow);
    leftButton->setArrowType(Qt::LeftArrow);
    rightButton->setArrowType(Qt::RightArrow);
    upButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    downButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    leftButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    rightButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
#else
    upButton = new QPushButton(tr("Up"));
    downButton = new QPushButton(tr("Down"));
    leftButton = new QPushButton("<");
    rightButton = new QPushButton(">");
#endif
    QVBoxLayout *buttonGrid = new QVBoxLayout;
    QHBoxLayout *upLayout = new QHBoxLayout;
    QHBoxLayout *inexcLayout = new QHBoxLayout;
    QHBoxLayout *downLayout = new QHBoxLayout;

    upLayout->addStretch();
    upLayout->addWidget(upButton);
    upLayout->addStretch();

    inexcLayout->addStretch();
    inexcLayout->addWidget(leftButton);
    inexcLayout->addWidget(rightButton);
    inexcLayout->addStretch();

    downLayout->addStretch();
    downLayout->addWidget(downButton);
    downLayout->addStretch();

    buttonGrid->addStretch();
    buttonGrid->addLayout(upLayout);
    buttonGrid->addLayout(inexcLayout);
    buttonGrid->addLayout(downLayout);
    buttonGrid->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addLayout(availLayout);
    hlayout->addLayout(buttonGrid);
    hlayout->addLayout(selectedLayout);
    setLayout(hlayout);

    QString s;
    if (appsettings->contains(GC_SETTINGS_BESTS_METRICS))
        s = appsettings->value(this, GC_SETTINGS_BESTS_METRICS).toString();
    else
        s = GC_SETTINGS_BESTS_METRICS_DEFAULT;
    QStringList selectedMetrics = s.split(",");

    const RideMetricFactory &factory = RideMetricFactory::instance();
    for (int i = 0; i < factory.metricCount(); ++i) {
        QString symbol = factory.metricName(i);
        if (selectedMetrics.contains(symbol) || symbol.startsWith("compatibility_"))
            continue;
        QSharedPointer<RideMetric> m(factory.newMetric(symbol));
        QListWidgetItem *item = new QListWidgetItem(Utils::unprotect(m->name()));
        item->setData(Qt::UserRole, symbol);
        item->setToolTip(m->description());
        availList->addItem(item);
    }
    foreach (QString symbol, selectedMetrics) {
        if (!factory.haveMetric(symbol))
            continue;
        QSharedPointer<RideMetric> m(factory.newMetric(symbol));
        QListWidgetItem *item = new QListWidgetItem(Utils::unprotect(m->name()));
        item->setData(Qt::UserRole, symbol);
        item->setToolTip(m->description());
        selectedList->addItem(item);
    }

    upButton->setEnabled(false);
    downButton->setEnabled(false);
    leftButton->setEnabled(false);
    rightButton->setEnabled(false);

    connect(upButton, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(downClicked()));
    connect(leftButton, SIGNAL(clicked()), this, SLOT(leftClicked()));
    connect(rightButton, SIGNAL(clicked()), this, SLOT(rightClicked()));
    connect(availList, SIGNAL(itemSelectionChanged()),
            this, SLOT(availChanged()));
    connect(selectedList, SIGNAL(itemSelectionChanged()),
            this, SLOT(selectedChanged()));
}

void
BestsMetricsPage::upClicked()
{
    assert(!selectedList->selectedItems().isEmpty());
    QListWidgetItem *item = selectedList->selectedItems().first();
    int row = selectedList->row(item);
    assert(row > 0);
    selectedList->takeItem(row);
    selectedList->insertItem(row - 1, item);
    selectedList->setCurrentItem(item);
    changed = true;
}

void
BestsMetricsPage::downClicked()
{
    assert(!selectedList->selectedItems().isEmpty());
    QListWidgetItem *item = selectedList->selectedItems().first();
    int row = selectedList->row(item);
    assert(row < selectedList->count() - 1);
    selectedList->takeItem(row);
    selectedList->insertItem(row + 1, item);
    selectedList->setCurrentItem(item);
    changed = true;
}

void
BestsMetricsPage::leftClicked()
{
    assert(!selectedList->selectedItems().isEmpty());
    QListWidgetItem *item = selectedList->selectedItems().first();
    selectedList->takeItem(selectedList->row(item));
    availList->addItem(item);
    changed = true;
    selectedChanged();
}

void
BestsMetricsPage::rightClicked()
{
    assert(!availList->selectedItems().isEmpty());
    QListWidgetItem *item = availList->selectedItems().first();
    availList->takeItem(availList->row(item));
    selectedList->addItem(item);
    changed = true;
}

void
BestsMetricsPage::availChanged()
{
    rightButton->setEnabled(!availList->selectedItems().isEmpty());
}

void
BestsMetricsPage::selectedChanged()
{
    if (selectedList->selectedItems().isEmpty()) {
        upButton->setEnabled(false);
        downButton->setEnabled(false);
        leftButton->setEnabled(false);
        return;
    }
    QListWidgetItem *item = selectedList->selectedItems().first();
    int row = selectedList->row(item);
    if (row == 0)
        upButton->setEnabled(false);
    else
        upButton->setEnabled(true);
    if (row == selectedList->count() - 1)
        downButton->setEnabled(false);
    else
        downButton->setEnabled(true);
    leftButton->setEnabled(true);
}

qint32
BestsMetricsPage::saveClicked()
{
    if (!changed) return 0;

    QStringList metrics;
    for (int i = 0; i < selectedList->count(); ++i)
        metrics << selectedList->item(i)->data(Qt::UserRole).toString();
    appsettings->setValue(GC_SETTINGS_BESTS_METRICS, metrics.join(","));

    return 0;
}

CustomMetricsPage::CustomMetricsPage(QWidget *parent, Context *context) :
    QWidget(parent), context(context)
{
    // copy as current, so we can edit...
    metrics = _userMetrics;
    b4.crc = RideMetric::userMetricFingerprint(metrics);

    table = new QTreeWidget;
    table->headerItem()->setText(0, tr("Symbol"));
    table->headerItem()->setText(1, tr("Name"));
    table->setColumnCount(2);
    table->setColumnWidth(0,200 *dpiXFactor);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setUniformRowHeights(true); // causes height problems when adding - in case of non-text fields
    table->setIndentation(0);

    refreshTable();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(table);
    connect(table, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(doubleClicked(QTreeWidgetItem*, int)));

    editButton = new QPushButton(tr("Edit"));
    exportButton = new QPushButton(tr("Export"));
    importButton = new QPushButton(tr("Import"));
#ifdef GC_HAS_CLOUD_DB
    uploadButton = new QPushButton(tr("Upload"));
    downloadButton = new QPushButton(tr("Download"));
#endif
    addButton = new QPushButton(tr("+"));
    deleteButton = new QPushButton(tr("-"));
#ifndef Q_OS_MAC
    addButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    deleteButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
#else
    addButton->setText(tr("Add"));
    deleteButton->setText(tr("Delete"));
#endif
    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addWidget(exportButton);
    buttons->addWidget(importButton);
    buttons->addStretch();
#ifdef GC_HAS_CLOUD_DB
    buttons->addWidget(uploadButton);
    buttons->addWidget(downloadButton);
    buttons->addStretch();
#endif
    buttons->addWidget(editButton);
    buttons->addStretch();
    buttons->addWidget(addButton);
    buttons->addWidget(deleteButton);

    layout->addLayout(buttons);

    connect(addButton, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editClicked()));
    connect(exportButton, SIGNAL(clicked()), this, SLOT(exportClicked()));
    connect(importButton, SIGNAL(clicked()), this, SLOT(importClicked()));
#ifdef GC_HAS_CLOUD_DB
    connect(uploadButton, SIGNAL(clicked()), this, SLOT(uploadClicked()));
    connect(downloadButton, SIGNAL(clicked()), this, SLOT(downloadClicked()));
#endif
}

void
CustomMetricsPage::refreshTable()
{
    table->clear();
    skipcompat=0;
    foreach(UserMetricSettings m, metrics) {

        if (m.symbol.startsWith("compatibility_")) {
            skipcompat++;
            continue;
        }

        // user metrics are silently discarded if the symbol is already in use
        if (!table->findItems(m.symbol, Qt::MatchExactly, 0).isEmpty())
            QMessageBox::warning(this, tr("User Metrics"), tr("Duplicate Symbol: %1, one metric will be discarded").arg(m.symbol));

        // duplicate names are allowed, but not recommended
        if (!table->findItems(m.name, Qt::MatchExactly, 1).isEmpty())
            QMessageBox::warning(this, tr("User Metrics"), tr("Duplicate Name: %1, one metric will not be acessible in formulas").arg(m.name));

        QTreeWidgetItem *add = new QTreeWidgetItem(table->invisibleRootItem());
        add->setText(0, m.symbol);
        add->setToolTip(0, m.description);
        add->setText(1, m.name);
        add->setToolTip(1, m.description);
    }
}

void
CustomMetricsPage::deleteClicked()
{
    // nothing selected
    if (table->selectedItems().count() <= 0) return;

    // which one?
    QTreeWidgetItem *item = table->selectedItems().first();
    int row = table->invisibleRootItem()->indexOfChild(item);

    // Are you sure ?
    QMessageBox msgBox;
    msgBox.setText(tr("Are you sure you want to delete this metric?"));
    msgBox.setInformativeText(metrics[row+skipcompat].name);
    QPushButton *deleteButton = msgBox.addButton(tr("Remove"),QMessageBox::YesRole);
    msgBox.setStandardButtons(QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();

    // nope, don't want to
    if(msgBox.clickedButton() != deleteButton) return;

    // wipe it away
    metrics.removeAt(row+skipcompat);

    // refresh
    refreshTable();
}

void
CustomMetricsPage::addClicked()
{
    UserMetricSettings here;

    // provide an example program to edit....
    here.symbol = "My_Average_Power";
    here.name = "My Average Power";
    here.type = 1;
    here.precision = 0;
    here.description = "Average Power computed using Joules to account for variable recording.";
    here.unitsMetric = "watts";
    here.unitsImperial = "watts";
    here.conversion = 1.00;
    here.conversionSum = 0.00;
    here.program ="{\n\
    # only calculate for rides containing power\n\
    relevant { Data contains \"P\"; }\n\
\n\
    # initialise aggregating variables\n\
    init { joules <- 0; seconds <- 0; }\n\
\n\
    # joules = power x time, for each sample\n\
    sample { \n\
        joules <- joules + (POWER * RECINTSECS);\n\
        seconds <- seconds + RECINTSECS;\n\
    }\n\
\n\
    # calculate metric value at end\n\
    value { joules / seconds; }\n\
    count { seconds; }\n\
}";

    EditUserMetricDialog editor(this, context, here);
    if (editor.exec() == QDialog::Accepted) {

        // add to the list
        metrics.append(here);
        refreshTable();

    }
}

void
CustomMetricsPage::editClicked()
{
    // nothing selected
    if (table->selectedItems().count() <= 0) return;

    // which one?
    QTreeWidgetItem *item = table->selectedItems().first();

    doubleClicked(item, 0);
}

void
CustomMetricsPage::doubleClicked(QTreeWidgetItem *item, int)
{
    // nothing selected
    if (item == NULL) return;

    // find row
    int row = table->invisibleRootItem()->indexOfChild(item);

    // edit it
    UserMetricSettings here = metrics[row+skipcompat];

    EditUserMetricDialog editor(this, context, here);
    if (editor.exec() == QDialog::Accepted) {

        // add to the list
        metrics[row+skipcompat] = here;
        refreshTable();

    }
}

void
CustomMetricsPage::exportClicked()
{
    // nothing selected
    if (table->selectedItems().count() <= 0) return;

    // which one?
    QTreeWidgetItem *item = table->selectedItems().first();

    // nothing selected
    if (item == NULL) return;

    // find row
    int row = table->invisibleRootItem()->indexOfChild(item);

    // metric to export
    UserMetricSettings here = metrics[row+skipcompat];

    // get a filename to export to...
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Metric"), QDir::homePath() + "/" + here.symbol + ".gmetric", tr("GoldenCheetah Metric File (*.gmetric)"));

    // nothing given
    if (filename.isEmpty()) return;

    UserMetricParser::serialize(filename, QList<UserMetricSettings>() << here);
}

void
CustomMetricsPage::importClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Metric file to import"), "", tr("GoldenCheetah Metric Files (*.gmetric)"));

    if (fileName.isEmpty()) {
        QMessageBox::critical(this, tr("Import Metric"), tr("No Metric file selected!"));
        return;
    }

    QList<UserMetricSettings> imported;
    QFile metricFile(fileName);

    // setup XML processor
    QXmlInputSource source( &metricFile );
    QXmlSimpleReader xmlReader;
    UserMetricParser handler;
    xmlReader.setContentHandler(&handler);
    xmlReader.setErrorHandler(&handler);

    // parse and get return values
    xmlReader.parse(source);
    imported = handler.getSettings();
    if (imported.isEmpty()) {
        QMessageBox::critical(this, tr("Import Metric"), tr("No Metric found in the selected file!"));
        return;
    }

    UserMetricSettings here = imported.first();

    EditUserMetricDialog editor(this, context, here);
    if (editor.exec() == QDialog::Accepted) {

        // add to the list
        metrics.append(here);
        refreshTable();

    }
}

#ifdef GC_HAS_CLOUD_DB
void
CustomMetricsPage::uploadClicked()
{
    // nothing selected
    if (table->selectedItems().count() <= 0) return;

    // which one?
    QTreeWidgetItem *item = table->selectedItems().first();

    // nothing selected
    if (item == NULL) return;

    // find row
    int row = table->invisibleRootItem()->indexOfChild(item);

    // metric to export
    UserMetricSettings here = metrics[row+skipcompat];

    // check for CloudDB T&C acceptance
    if (!(appsettings->cvalue(context->athlete->cyclist, GC_CLOUDDB_TC_ACCEPTANCE, false).toBool())) {
        CloudDBAcceptConditionsDialog acceptDialog(context->athlete->cyclist);
        acceptDialog.setModal(true);
        if (acceptDialog.exec() == QDialog::Rejected) {
            return;
        }
    }

    UserMetricAPIv1 usermetric;
    usermetric.Header.Key = here.symbol;
    usermetric.Header.Name = here.name;
    usermetric.Header.Description = here.description;
    int version = VERSION_LATEST;
    usermetric.Header.GcVersion =  QString::number(version);
    // get the usermetric - definition xml
    QTextStream out(&usermetric.UserMetricXML);
    UserMetricParser::serializeToQTextStream(out, QList<UserMetricSettings>() << here);

    usermetric.Header.CreatorId = appsettings->cvalue(context->athlete->cyclist, GC_ATHLETE_ID, "").toString();
    usermetric.Header.Curated = false;
    usermetric.Header.Deleted = false;

    // now complete the usermetric with for the user manually added fields
    CloudDBUserMetricObjectDialog dialog(usermetric, context->athlete->cyclist);
    if (dialog.exec() == QDialog::Accepted) {
        CloudDBUserMetricClient c;
        if (c.postUserMetric(dialog.getUserMetric())) {
            CloudDBHeader::setUserMetricHeaderStale(true);
        }
    }

}

void
CustomMetricsPage::downloadClicked()
{
    if (!(appsettings->cvalue(context->athlete->cyclist, GC_CLOUDDB_TC_ACCEPTANCE, false).toBool())) {
       CloudDBAcceptConditionsDialog acceptDialog(context->athlete->cyclist);
       acceptDialog.setModal(true);
       if (acceptDialog.exec() == QDialog::Rejected) {
          return;
       }
    }

    if (context->cdbUserMetricListDialog == NULL) {
        context->cdbUserMetricListDialog = new CloudDBUserMetricListDialog();
    }

    if (context->cdbUserMetricListDialog->prepareData(context->athlete->cyclist, CloudDBCommon::UserImport)) {
        if (context->cdbUserMetricListDialog->exec() == QDialog::Accepted) {

            QList<QString> usermetricDefs = context->cdbUserMetricListDialog->getSelectedSettings();

            foreach (QString usermetricDef, usermetricDefs) {
                QList<UserMetricSettings> imported;

                // setup XML processor
                QXmlInputSource source;
                source.setData(usermetricDef);
                QXmlSimpleReader xmlReader;
                UserMetricParser handler;
                xmlReader.setContentHandler(&handler);
                xmlReader.setErrorHandler(&handler);

                // parse and get return values
                xmlReader.parse(source);
                imported = handler.getSettings();
                if (imported.isEmpty()) {
                    QMessageBox::critical(this, tr("Download Metric"), tr("No valid Metric found!"));
                    continue;
                }

                UserMetricSettings here = imported.first();

                EditUserMetricDialog editor(this, context, here);
                if (editor.exec() == QDialog::Accepted) {

                    // add to the list
                    metrics.append(here);
                    refreshTable();

                }
            }
        }
    }
}
#endif

qint32
CustomMetricsPage::saveClicked()
{
    qint32 returning=0;

    // did we actually change them ?
    if (b4.crc != RideMetric::userMetricFingerprint(metrics))
        returning |= CONFIG_USERMETRICS;

    // save away OUR version to top-level NOT athlete dir
    // and don't overwrite in memory version the signal handles that
    QString filename = QString("%1/../usermetrics.xml").arg(context->athlete->home->root().absolutePath());
    UserMetricParser::serialize(filename, metrics);

    // the change will need to be signalled by context, not us
    return returning;
}

SummaryMetricsPage::SummaryMetricsPage(QWidget *parent) :
    QWidget(parent), changed(false)
{
    HelpWhatsThis *help = new HelpWhatsThis(this);
    this->setWhatsThis(help->getWhatsThisText(HelpWhatsThis::Preferences_Metrics_Summary));

    availList = new QListWidget;
    availList->setSortingEnabled(true);
    availList->setSelectionMode(QAbstractItemView::SingleSelection);
    QVBoxLayout *availLayout = new QVBoxLayout;
    availLayout->addWidget(new QLabel(tr("Available Metrics")));
    availLayout->addWidget(availList);
    selectedList = new QListWidget;
    selectedList->setSelectionMode(QAbstractItemView::SingleSelection);
    QVBoxLayout *selectedLayout = new QVBoxLayout;
    selectedLayout->addWidget(new QLabel(tr("Selected Metrics")));
    selectedLayout->addWidget(selectedList);
#ifndef Q_OS_MAC
    upButton = new QToolButton(this);
    downButton = new QToolButton(this);
    leftButton = new QToolButton(this);
    rightButton = new QToolButton(this);
    upButton->setArrowType(Qt::UpArrow);
    downButton->setArrowType(Qt::DownArrow);
    leftButton->setArrowType(Qt::LeftArrow);
    rightButton->setArrowType(Qt::RightArrow);
    upButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    downButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    leftButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    rightButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
#else
    upButton = new QPushButton(tr("Up"));
    downButton = new QPushButton(tr("Down"));
    leftButton = new QPushButton("<");
    rightButton = new QPushButton(">");
#endif
    QVBoxLayout *buttonGrid = new QVBoxLayout;
    QHBoxLayout *upLayout = new QHBoxLayout;
    QHBoxLayout *inexcLayout = new QHBoxLayout;
    QHBoxLayout *downLayout = new QHBoxLayout;

    upLayout->addStretch();
    upLayout->addWidget(upButton);
    upLayout->addStretch();

    inexcLayout->addStretch();
    inexcLayout->addWidget(leftButton);
    inexcLayout->addWidget(rightButton);
    inexcLayout->addStretch();

    downLayout->addStretch();
    downLayout->addWidget(downButton);
    downLayout->addStretch();

    buttonGrid->addStretch();
    buttonGrid->addLayout(upLayout);
    buttonGrid->addLayout(inexcLayout);
    buttonGrid->addLayout(downLayout);
    buttonGrid->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addLayout(availLayout);
    hlayout->addLayout(buttonGrid);
    hlayout->addLayout(selectedLayout);
    setLayout(hlayout);

    QString s = appsettings->value(this, GC_SETTINGS_SUMMARY_METRICS, GC_SETTINGS_SUMMARY_METRICS_DEFAULT).toString();
    QStringList selectedMetrics = s.split(",");

    const RideMetricFactory &factory = RideMetricFactory::instance();
    for (int i = 0; i < factory.metricCount(); ++i) {
        QString symbol = factory.metricName(i);
        if (selectedMetrics.contains(symbol) || symbol.startsWith("compatibility_"))
            continue;
        QSharedPointer<RideMetric> m(factory.newMetric(symbol));
        QListWidgetItem *item = new QListWidgetItem(Utils::unprotect(m->name()));
        item->setData(Qt::UserRole, symbol);
        item->setToolTip(m->description());
        availList->addItem(item);
    }
    foreach (QString symbol, selectedMetrics) {
        if (!factory.haveMetric(symbol))
            continue;
        QSharedPointer<RideMetric> m(factory.newMetric(symbol));
        QListWidgetItem *item = new QListWidgetItem(Utils::unprotect(m->name()));
        item->setData(Qt::UserRole, symbol);
        item->setToolTip(m->description());
        selectedList->addItem(item);
    }

    upButton->setEnabled(false);
    downButton->setEnabled(false);
    leftButton->setEnabled(false);
    rightButton->setEnabled(false);

    connect(upButton, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(downClicked()));
    connect(leftButton, SIGNAL(clicked()), this, SLOT(leftClicked()));
    connect(rightButton, SIGNAL(clicked()), this, SLOT(rightClicked()));
    connect(availList, SIGNAL(itemSelectionChanged()),
            this, SLOT(availChanged()));
    connect(selectedList, SIGNAL(itemSelectionChanged()),
            this, SLOT(selectedChanged()));
}

void
SummaryMetricsPage::upClicked()
{
    assert(!selectedList->selectedItems().isEmpty());
    QListWidgetItem *item = selectedList->selectedItems().first();
    int row = selectedList->row(item);
    assert(row > 0);
    selectedList->takeItem(row);
    selectedList->insertItem(row - 1, item);
    selectedList->setCurrentItem(item);
    changed = true;
}

void
SummaryMetricsPage::downClicked()
{
    assert(!selectedList->selectedItems().isEmpty());
    QListWidgetItem *item = selectedList->selectedItems().first();
    int row = selectedList->row(item);
    assert(row < selectedList->count() - 1);
    selectedList->takeItem(row);
    selectedList->insertItem(row + 1, item);
    selectedList->setCurrentItem(item);
    changed = true;
}

void
SummaryMetricsPage::leftClicked()
{
    assert(!selectedList->selectedItems().isEmpty());
    QListWidgetItem *item = selectedList->selectedItems().first();
    selectedList->takeItem(selectedList->row(item));
    availList->addItem(item);
    changed = true;
    selectedChanged();
}

void
SummaryMetricsPage::rightClicked()
{
    assert(!availList->selectedItems().isEmpty());
    QListWidgetItem *item = availList->selectedItems().first();
    availList->takeItem(availList->row(item));
    selectedList->addItem(item);
    changed = true;
}

void
SummaryMetricsPage::availChanged()
{
    rightButton->setEnabled(!availList->selectedItems().isEmpty());
}

void
SummaryMetricsPage::selectedChanged()
{
    if (selectedList->selectedItems().isEmpty()) {
        upButton->setEnabled(false);
        downButton->setEnabled(false);
        leftButton->setEnabled(false);
        return;
    }
    QListWidgetItem *item = selectedList->selectedItems().first();
    int row = selectedList->row(item);
    if (row == 0)
        upButton->setEnabled(false);
    else
        upButton->setEnabled(true);
    if (row == selectedList->count() - 1)
        downButton->setEnabled(false);
    else
        downButton->setEnabled(true);
    leftButton->setEnabled(true);
}

qint32
SummaryMetricsPage::saveClicked()
{
    if (!changed) return 0;

    QStringList metrics;
    for (int i = 0; i < selectedList->count(); ++i)
        metrics << selectedList->item(i)->data(Qt::UserRole).toString();
    appsettings->setValue(GC_SETTINGS_SUMMARY_METRICS, metrics.join(","));

    return 0;
}

MetadataPage::MetadataPage(Context *context) : context(context)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // get current config using default file
    keywordDefinitions = GlobalContext::context()->rideMetadata->getKeywords();
    fieldDefinitions = GlobalContext::context()->rideMetadata->getFields();
    colorfield = GlobalContext::context()->rideMetadata->getColorField();
    defaultDefinitions = GlobalContext::context()->rideMetadata->getDefaults();

    // setup maintenance pages using current config
    fieldsPage = new FieldsPage(this, fieldDefinitions);
    keywordsPage = new KeywordsPage(this, keywordDefinitions);
    defaultsPage = new DefaultsPage(this, defaultDefinitions);
    processorPage = new ProcessorPage(context);


    tabs = new QTabWidget(this);
    tabs->addTab(fieldsPage, tr("Fields"));
    tabs->addTab(keywordsPage, tr("Colour Keywords"));
    tabs->addTab(defaultsPage, tr("Defaults"));
    tabs->addTab(processorPage, tr("Processing"));


    // refresh the keywords combo when change tabs .. will do more often than
    // needed but better that than less often than needed
    connect (tabs, SIGNAL(currentChanged(int)), keywordsPage, SLOT(pageSelected()));

    layout->addWidget(tabs);

    // save initial values
    b4.keywordFingerprint = KeywordDefinition::fingerprint(keywordDefinitions);
    b4.colorfield = colorfield;
    b4.fieldFingerprint = FieldDefinition::fingerprint(fieldDefinitions);
}

qint32
MetadataPage::saveClicked()
{
    // get current state
    fieldsPage->getDefinitions(fieldDefinitions);
    keywordsPage->getDefinitions(keywordDefinitions);
    defaultsPage->getDefinitions(defaultDefinitions);

    // save settings
    appsettings->setValue(GC_RIDEBG, keywordsPage->rideBG->isChecked());

    // write to metadata.xml
    RideMetadata::serialize(QDir(gcroot).canonicalPath() + "/metadata.xml", keywordDefinitions, fieldDefinitions, colorfield, defaultDefinitions);

    // save processors config
    processorPage->saveClicked();

    qint32 state = 0;

    if (b4.keywordFingerprint != KeywordDefinition::fingerprint(keywordDefinitions) || b4.colorfield != colorfield)
        state += CONFIG_NOTECOLOR;

    if (b4.fieldFingerprint != FieldDefinition::fingerprint(fieldDefinitions))
        state += CONFIG_FIELDS;

    return state;
}

// little helper since we create/recreate combos
// for field types all over the place (init, move up, move down)
void
FieldsPage::addFieldTypes(QComboBox *p)
{
    p->addItem(tr("Text"));
    p->addItem(tr("Textbox"));
    p->addItem(tr("ShortText"));
    p->addItem(tr("Integer"));
    p->addItem(tr("Double"));
    p->addItem(tr("Date"));
    p->addItem(tr("Time"));
    p->addItem(tr("Checkbox"));
}

//
// Calendar coloring page
//
KeywordsPage::KeywordsPage(MetadataPage *parent, QList<KeywordDefinition>keywordDefinitions) : QWidget(parent), parent(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    HelpWhatsThis *help = new HelpWhatsThis(this);
    this->setWhatsThis(help->getWhatsThisText(HelpWhatsThis::Preferences_DataFields_Notes_Keywords));

    QHBoxLayout *field = new QHBoxLayout();
    fieldLabel = new QLabel(tr("Field"),this);
    fieldChooser = new QComboBox(this);
    field->addWidget(fieldLabel);
    field->addWidget(fieldChooser);
    field->addStretch();
    mainLayout->addLayout(field);

    rideBG = new QCheckBox(tr("Use for Background"));
    rideBG->setChecked(appsettings->value(this, GC_RIDEBG, false).toBool());
    field->addWidget(rideBG);

    addButton = new QPushButton(tr("+"));
    deleteButton = new QPushButton(tr("-"));
#ifndef Q_OS_MAC
    upButton = new QToolButton(this);
    downButton = new QToolButton(this);
    upButton->setArrowType(Qt::UpArrow);
    downButton->setArrowType(Qt::DownArrow);
    upButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    downButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    addButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    deleteButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
#else
    addButton->setText(tr("Add"));
    deleteButton->setText(tr("Delete"));
    upButton = new QPushButton(tr("Up"));
    downButton = new QPushButton(tr("Down"));
#endif

    QHBoxLayout *actionButtons = new QHBoxLayout;
    actionButtons->setSpacing(2 *dpiXFactor);
    actionButtons->addWidget(upButton);
    actionButtons->addWidget(downButton);
    actionButtons->addStretch();
    actionButtons->addWidget(addButton);
    actionButtons->addWidget(deleteButton);

    keywords = new QTreeWidget;
    keywords->headerItem()->setText(0, tr("Keyword"));
    keywords->headerItem()->setText(1, tr("Color"));
    keywords->headerItem()->setText(2, tr("Related Notes Words"));
    keywords->setColumnCount(3);
    keywords->setSelectionMode(QAbstractItemView::SingleSelection);
    keywords->setEditTriggers(QAbstractItemView::SelectedClicked); // allow edit
    //keywords->setUniformRowHeights(true); // causes height problems when adding - in case of non-text fields
    keywords->setIndentation(0);
    //keywords->header()->resizeSection(0,100);
    //keywords->header()->resizeSection(1,45);

    foreach(KeywordDefinition keyword, keywordDefinitions) {
        QTreeWidgetItem *add;
        ColorButton *colorButton = new ColorButton(this, keyword.name, keyword.color);
        add = new QTreeWidgetItem(keywords->invisibleRootItem());
        add->setFlags(add->flags() | Qt::ItemIsEditable);

        // keyword
        add->setText(0, keyword.name);

        // color button
        add->setTextAlignment(1, Qt::AlignHCenter);
        keywords->setItemWidget(add, 1, colorButton);

        QString text;
        for (int i=0; i< keyword.tokens.count(); i++) {
            if (i != keyword.tokens.count()-1)
                text += keyword.tokens[i] + ",";
            else
                text += keyword.tokens[i];
        }

        // notes texts
        add->setText(2, text);
    }
    keywords->setCurrentItem(keywords->invisibleRootItem()->child(0));

    mainLayout->addWidget(keywords);
    mainLayout->addLayout(actionButtons);

    // connect up slots
    connect(upButton, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(downClicked()));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));

    connect(fieldChooser, SIGNAL(currentIndexChanged(int)), this, SLOT(colorfieldChanged()));
}

void
KeywordsPage::pageSelected()
{
    SpecialFields sp;
    QString prev = "";

    // remember what was selected, if anything?
    if (fieldChooser->count()) {
        prev = sp.internalName(fieldChooser->itemText(fieldChooser->currentIndex()));
        parent->colorfield = prev;
    } else prev = parent->colorfield;
    // load in texts from metadata
    fieldChooser->clear();

    // get the current fields definitions
    QList<FieldDefinition> fromFieldsPage;
    parent->fieldsPage->getDefinitions(fromFieldsPage);
    foreach(FieldDefinition x, fromFieldsPage) {
        if (x.type < 3) fieldChooser->addItem(sp.displayName(x.name));
    }
    fieldChooser->setCurrentIndex(fieldChooser->findText(sp.displayName(prev)));
}

void
KeywordsPage::colorfieldChanged()
{
    SpecialFields sp;
    int index = fieldChooser->currentIndex();
    if (index >=0) parent->colorfield = sp.internalName(fieldChooser->itemText(fieldChooser->currentIndex()));
}


void
KeywordsPage::upClicked()
{
    if (keywords->currentItem()) {
        int index = keywords->invisibleRootItem()->indexOfChild(keywords->currentItem());
        if (index == 0) return; // its at the top already

        // movin on up!
        QWidget *button = keywords->itemWidget(keywords->currentItem(),1);
        ColorButton *colorButton = new ColorButton(this, ((ColorButton*)button)->getName(), ((ColorButton*)button)->getColor());
        QTreeWidgetItem* moved = keywords->invisibleRootItem()->takeChild(index);
        keywords->invisibleRootItem()->insertChild(index-1, moved);
        keywords->setItemWidget(moved, 1, colorButton);
        keywords->setCurrentItem(moved);
        //LTMSettings save = (*presets)[index];
        //presets->removeAt(index);
        //presets->insert(index-1, save);
    }
}

void
KeywordsPage::downClicked()
{
    if (keywords->currentItem()) {
        int index = keywords->invisibleRootItem()->indexOfChild(keywords->currentItem());
        if (index == (keywords->invisibleRootItem()->childCount()-1)) return; // its at the bottom already

        // movin on up!
        QWidget *button = keywords->itemWidget(keywords->currentItem(),1);
        ColorButton *colorButton = new ColorButton(this, ((ColorButton*)button)->getName(), ((ColorButton*)button)->getColor());
        QTreeWidgetItem* moved = keywords->invisibleRootItem()->takeChild(index);
        keywords->invisibleRootItem()->insertChild(index+1, moved);
        keywords->setItemWidget(moved, 1, colorButton);
        keywords->setCurrentItem(moved);
    }
}

void
KeywordsPage::renameClicked()
{
    // which one is selected?
    if (keywords->currentItem()) keywords->editItem(keywords->currentItem(), 0);
}

void
KeywordsPage::addClicked()
{
    int index = keywords->invisibleRootItem()->indexOfChild(keywords->currentItem());
    if (index < 0) index = 0;
    QTreeWidgetItem *add;
    ColorButton *colorButton = new ColorButton(this, tr("New"), QColor(Qt::blue));
    add = new QTreeWidgetItem;
    keywords->invisibleRootItem()->insertChild(index, add);
    add->setFlags(add->flags() | Qt::ItemIsEditable);

    // keyword
    QString text = tr("New");
    for (int i=0; keywords->findItems(text, Qt::MatchExactly, 0).count() > 0; i++) {
        text = QString(tr("New (%1)")).arg(i+1);
    }
    add->setText(0, text);

    // color button
    add->setTextAlignment(1, Qt::AlignHCenter);
    keywords->setItemWidget(add, 1, colorButton);

    // notes texts
    add->setText(2, "");
}

void
KeywordsPage::deleteClicked()
{
    if (keywords->currentItem()) {
        int index = keywords->invisibleRootItem()->indexOfChild(keywords->currentItem());

        // zap!
        delete keywords->invisibleRootItem()->takeChild(index);
    }
}

void
KeywordsPage::getDefinitions(QList<KeywordDefinition> &keywordList)
{
    // clear current just in case
    keywordList.clear();

    for (int idx =0; idx < keywords->invisibleRootItem()->childCount(); idx++) {
        KeywordDefinition add;
        QTreeWidgetItem *item = keywords->invisibleRootItem()->child(idx);

        add.name = item->text(0);
        add.color = ((ColorButton*)keywords->itemWidget(item, 1))->getColor();
        add.tokens = item->text(2).split(",", QString::SkipEmptyParts);

        keywordList.append(add);
    }
}

//
// Ride metadata page
//
FieldsPage::FieldsPage(QWidget *parent, QList<FieldDefinition>fieldDefinitions) : QWidget(parent)
{
    QGridLayout *mainLayout = new QGridLayout(this);
    HelpWhatsThis *help = new HelpWhatsThis(this);
    this->setWhatsThis(help->getWhatsThisText(HelpWhatsThis::Preferences_DataFields_Fields));

    addButton = new QPushButton(tr("+"));
    deleteButton = new QPushButton(tr("-"));
#ifndef Q_OS_MAC
    upButton = new QToolButton(this);
    downButton = new QToolButton(this);
    upButton->setArrowType(Qt::UpArrow);
    downButton->setArrowType(Qt::DownArrow);
    upButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    downButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    addButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    deleteButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
#else
    addButton->setText(tr("Add"));
    deleteButton->setText(tr("Delete"));
    upButton = new QPushButton(tr("Up"));
    downButton = new QPushButton(tr("Down"));
#endif
    QHBoxLayout *actionButtons = new QHBoxLayout;
    actionButtons->setSpacing(2 *dpiXFactor);
    actionButtons->addWidget(upButton);
    actionButtons->addWidget(downButton);
    actionButtons->addStretch();
    actionButtons->addWidget(addButton);
    actionButtons->addWidget(deleteButton);

    fields = new QTreeWidget;
    fields->headerItem()->setText(0, tr("Screen Tab"));
    fields->headerItem()->setText(1, tr("Field"));
    fields->headerItem()->setText(2, tr("Type"));
    fields->headerItem()->setText(3, tr("Values"));
    fields->headerItem()->setText(4, tr("Diary"));
    fields->setColumnWidth(0,80 *dpiXFactor);
    fields->setColumnWidth(1,100 *dpiXFactor);
    fields->setColumnWidth(2,100 *dpiXFactor);
    fields->setColumnWidth(3,80 *dpiXFactor);
    fields->setColumnWidth(4,20 *dpiXFactor);
    fields->setColumnCount(5);
    fields->setSelectionMode(QAbstractItemView::SingleSelection);
    fields->setEditTriggers(QAbstractItemView::SelectedClicked); // allow edit
    //fields->setUniformRowHeights(true); // causes height problems when adding - in case of non-text fields
    fields->setIndentation(0);

    SpecialFields specials;
    SpecialTabs specialTabs;
    foreach(FieldDefinition field, fieldDefinitions) {
        QTreeWidgetItem *add;
        QComboBox *comboButton = new QComboBox(this);
        QCheckBox *checkBox = new QCheckBox("", this);
        checkBox->setChecked(field.diary);

        addFieldTypes(comboButton);
        comboButton->setCurrentIndex(field.type);

        add = new QTreeWidgetItem(fields->invisibleRootItem());
        add->setFlags(add->flags() | Qt::ItemIsEditable);

        // tab name
        add->setText(0, specialTabs.displayName(field.tab));
        // field name
        add->setText(1, specials.displayName(field.name));
        // values
        add->setText(3, field.values.join(","));

        // type button
        add->setTextAlignment(2, Qt::AlignHCenter);
        fields->setItemWidget(add, 2, comboButton);
        fields->setItemWidget(add, 4, checkBox);
    }
    fields->setCurrentItem(fields->invisibleRootItem()->child(0));

    mainLayout->addWidget(fields, 0,0);
    mainLayout->addLayout(actionButtons, 1,0);

    // connect up slots
    connect(upButton, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(downClicked()));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));
}

void
FieldsPage::upClicked()
{
    if (fields->currentItem()) {
        int index = fields->invisibleRootItem()->indexOfChild(fields->currentItem());
        if (index == 0) return; // its at the top already

        // movin on up!
        QWidget *button = fields->itemWidget(fields->currentItem(),2);
        QWidget *check = fields->itemWidget(fields->currentItem(),4);
        QComboBox *comboButton = new QComboBox(this);
        addFieldTypes(comboButton);
        comboButton->setCurrentIndex(((QComboBox*)button)->currentIndex());
        QCheckBox *checkBox = new QCheckBox("", this);
        checkBox->setChecked(((QCheckBox*)check)->isChecked());
        QTreeWidgetItem* moved = fields->invisibleRootItem()->takeChild(index);
        fields->invisibleRootItem()->insertChild(index-1, moved);
        fields->setItemWidget(moved, 2, comboButton);
        fields->setItemWidget(moved, 4, checkBox);
        fields->setCurrentItem(moved);
    }
}

void
FieldsPage::downClicked()
{
    if (fields->currentItem()) {
        int index = fields->invisibleRootItem()->indexOfChild(fields->currentItem());
        if (index == (fields->invisibleRootItem()->childCount()-1)) return; // its at the bottom already

        QWidget *button = fields->itemWidget(fields->currentItem(),2);
        QWidget *check = fields->itemWidget(fields->currentItem(),4);
        QComboBox *comboButton = new QComboBox(this);
        addFieldTypes(comboButton);
        comboButton->setCurrentIndex(((QComboBox*)button)->currentIndex());
        QCheckBox *checkBox = new QCheckBox("", this);
        checkBox->setChecked(((QCheckBox*)check)->isChecked());
        QTreeWidgetItem* moved = fields->invisibleRootItem()->takeChild(index);
        fields->invisibleRootItem()->insertChild(index+1, moved);
        fields->setItemWidget(moved, 2, comboButton);
        fields->setItemWidget(moved, 4, checkBox);
        fields->setCurrentItem(moved);
    }
}

void
FieldsPage::renameClicked()
{
    // which one is selected?
    if (fields->currentItem()) fields->editItem(fields->currentItem(), 0);
}

void
FieldsPage::addClicked()
{
    int index = fields->invisibleRootItem()->indexOfChild(fields->currentItem());
    if (index < 0) index = 0;
    QTreeWidgetItem *add;
    QComboBox *comboButton = new QComboBox(this);
    addFieldTypes(comboButton);
    QCheckBox *checkBox = new QCheckBox("", this);

    add = new QTreeWidgetItem;
    fields->invisibleRootItem()->insertChild(index, add);
    add->setFlags(add->flags() | Qt::ItemIsEditable);

    // field
    QString text = tr("New");
    for (int i=0; fields->findItems(text, Qt::MatchExactly, 1).count() > 0; i++) {
        text = QString(tr("New (%1)")).arg(i+1);
    }
    add->setText(1, text);

    // type button
    add->setTextAlignment(2, Qt::AlignHCenter);
    fields->setItemWidget(add, 2, comboButton);
    fields->setItemWidget(add, 4, checkBox);
}

void
FieldsPage::deleteClicked()
{
    if (fields->currentItem()) {
        int index = fields->invisibleRootItem()->indexOfChild(fields->currentItem());

        // zap!
        delete fields->invisibleRootItem()->takeChild(index);
    }
}

void
FieldsPage::getDefinitions(QList<FieldDefinition> &fieldList)
{
    SpecialFields sp;
    SpecialTabs st;
    QStringList checkdups;

    // clear current just in case
    fieldList.clear();

    for (int idx =0; idx < fields->invisibleRootItem()->childCount(); idx++) {

        FieldDefinition add;
        QTreeWidgetItem *item = fields->invisibleRootItem()->child(idx);

        // silently ignore duplicates
        if (checkdups.contains(item->text(1))) continue;
        else checkdups << item->text(1);

        add.tab = st.internalName(item->text(0));
        add.name = sp.internalName(item->text(1));
        add.values = item->text(3).split(QRegExp("(, *|,)"), QString::KeepEmptyParts);
        add.diary = ((QCheckBox*)fields->itemWidget(item, 4))->isChecked();

        if (sp.isMetric(add.name))
            add.type = 4;
        else
            add.type = ((QComboBox*)fields->itemWidget(item, 2))->currentIndex();

        fieldList.append(add);
    }
}

//
// Data processors config page
//
ProcessorPage::ProcessorPage(Context *context) : context(context)
{
    // get the available processors
    const DataProcessorFactory &factory = DataProcessorFactory::instance();
    processors = factory.getProcessors();

    QGridLayout *mainLayout = new QGridLayout(this);

    processorTree = new QTreeWidget;
    processorTree->headerItem()->setText(0, tr("Processor"));
    processorTree->headerItem()->setText(1, tr("Apply"));
    processorTree->headerItem()->setText(2, tr("Settings"));
    processorTree->headerItem()->setText(3, "Technical Name of Processor"); // add invisible Column with technical name
    processorTree->setColumnCount(4);
    processorTree->setSelectionMode(QAbstractItemView::NoSelection);
    processorTree->setEditTriggers(QAbstractItemView::SelectedClicked); // allow edit
    processorTree->setUniformRowHeights(true);
    processorTree->setIndentation(0);
    //processorTree->header()->resizeSection(0,150);
    HelpWhatsThis *help = new HelpWhatsThis(processorTree);
    processorTree->setWhatsThis(help->getWhatsThisText(HelpWhatsThis::Preferences_DataFields_Processing));

    // iterate over all the processors and add an entry to the
    QMapIterator<QString, DataProcessor*> i(processors);
    i.toFront();
    while (i.hasNext()) {
        i.next();

        QTreeWidgetItem *add;

        add = new QTreeWidgetItem(processorTree->invisibleRootItem());
        add->setFlags(add->flags() & ~Qt::ItemIsEditable);

        // Processor Name: it shows the localized name
        add->setText(0, i.value()->name());
        // Processer Key - for Settings
        add->setText(3, i.key());

        // Auto or Manual run?
        QComboBox *comboButton = new QComboBox(this);
        comboButton->addItem(tr("Manual"));
        comboButton->addItem(tr("Import"));
        comboButton->addItem(tr("Save"));
        processorTree->setItemWidget(add, 1, comboButton);

        QString configsetting = QString("dp/%1/apply").arg(i.key());
        if (appsettings->value(NULL, GC_QSETTINGS_GLOBAL_GENERAL+configsetting, "Manual").toString() == "Manual")
            comboButton->setCurrentIndex(0);
        else if (appsettings->value(NULL, GC_QSETTINGS_GLOBAL_GENERAL+configsetting, "Save").toString() == "Save")
            comboButton->setCurrentIndex(2);
        else
            comboButton->setCurrentIndex(1);

        // Get and Set the Config Widget
        DataProcessorConfig *config = i.value()->processorConfig(this);
        config->readConfig();

        processorTree->setItemWidget(add, 2, config);

    }
    processorTree->setColumnHidden(3, true);

    mainLayout->addWidget(processorTree, 0,0);
}

qint32
ProcessorPage::saveClicked()
{
    // call each processor config widget's saveConfig() to
    // write away separately
    for (int i=0; i<processorTree->invisibleRootItem()->childCount(); i++) {
        // auto (which means run on import) or manual?
        QString configsetting = QString("dp/%1/apply").arg(processorTree->invisibleRootItem()->child(i)->text(3));
        QString apply;

        // which mode is selected?
        switch(((QComboBox*)(processorTree->itemWidget(processorTree->invisibleRootItem()->child(i), 1)))->currentIndex())  {
            default:
            case 0: apply = "Manual"; break;
            case 1: apply = "Auto"; break;
            case 2: apply = "Save"; break;
        }

        appsettings->setValue(GC_QSETTINGS_GLOBAL_GENERAL+configsetting, apply);
        ((DataProcessorConfig*)(processorTree->itemWidget(processorTree->invisibleRootItem()->child(i), 2)))->saveConfig();
    }

    return 0;
}

//
// Default values page
//
DefaultsPage::DefaultsPage(QWidget *parent, QList<DefaultDefinition>defaultDefinitions) : QWidget(parent)
{
    QGridLayout *mainLayout = new QGridLayout(this);
    HelpWhatsThis *help = new HelpWhatsThis(this);
    this->setWhatsThis(help->getWhatsThisText(HelpWhatsThis::Preferences_DataFields_Defaults));

    addButton = new QPushButton(tr("+"));
    deleteButton = new QPushButton(tr("-"));
#ifndef Q_OS_MAC
    upButton = new QToolButton(this);
    downButton = new QToolButton(this);
    upButton->setArrowType(Qt::UpArrow);
    downButton->setArrowType(Qt::DownArrow);
    upButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    downButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    addButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
    deleteButton->setFixedSize(20*dpiXFactor,20*dpiYFactor);
#else
    addButton->setText(tr("Add"));
    deleteButton->setText(tr("Delete"));
    upButton = new QPushButton(tr("Up"));
    downButton = new QPushButton(tr("Down"));
#endif
    QHBoxLayout *actionButtons = new QHBoxLayout;
    actionButtons->setSpacing(2 *dpiXFactor);
    actionButtons->addWidget(upButton);
    actionButtons->addWidget(downButton);
    actionButtons->addStretch();
    actionButtons->addWidget(addButton);
    actionButtons->addWidget(deleteButton);

    defaults = new QTreeWidget;
    defaults->headerItem()->setText(0, tr("Field"));
    defaults->headerItem()->setText(1, tr("Value"));
    defaults->headerItem()->setText(2, tr("Linked field"));
    defaults->headerItem()->setText(3, tr("Default Value"));
    defaults->setColumnWidth(0,80 *dpiXFactor);
    defaults->setColumnWidth(1,100 *dpiXFactor);
    defaults->setColumnWidth(2,80 *dpiXFactor);
    defaults->setColumnWidth(3,100 *dpiXFactor);
    defaults->setColumnCount(4);
    defaults->setSelectionMode(QAbstractItemView::SingleSelection);
    defaults->setEditTriggers(QAbstractItemView::SelectedClicked); // allow edit
    //defaults->setUniformRowHeights(true); // causes height problems when adding - in case of non-text fields
    defaults->setIndentation(0);

    SpecialFields specials;
    foreach(DefaultDefinition adefault, defaultDefinitions) {
        QTreeWidgetItem *add;

        add = new QTreeWidgetItem(defaults->invisibleRootItem());
        add->setFlags(add->flags() | Qt::ItemIsEditable);

        // field name
        add->setText(0, specials.displayName(adefault.field));
        // value
        add->setText(1, adefault.value);
        // Linked field
        add->setText(2, adefault.linkedField);
        // Default Value
        add->setText(3, adefault.linkedValue);
    }
    defaults->setCurrentItem(defaults->invisibleRootItem()->child(0));

    mainLayout->addWidget(defaults, 0,0);
    mainLayout->addLayout(actionButtons, 1,0);

    // connect up slots
    connect(upButton, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(downClicked()));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));
}

void
DefaultsPage::upClicked()
{
    if (defaults->currentItem()) {
        int index = defaults->invisibleRootItem()->indexOfChild(defaults->currentItem());
        if (index == 0) return; // its at the top already

        QTreeWidgetItem* moved = defaults->invisibleRootItem()->takeChild(index);
        defaults->invisibleRootItem()->insertChild(index-1, moved);
        defaults->setCurrentItem(moved);
    }
}

void
DefaultsPage::downClicked()
{
    if (defaults->currentItem()) {
        int index = defaults->invisibleRootItem()->indexOfChild(defaults->currentItem());
        if (index == (defaults->invisibleRootItem()->childCount()-1)) return; // its at the bottom already

        QTreeWidgetItem* moved = defaults->invisibleRootItem()->takeChild(index);
        defaults->invisibleRootItem()->insertChild(index+1, moved);
        defaults->setCurrentItem(moved);
    }
}

void
DefaultsPage::addClicked()
{
    int index = defaults->invisibleRootItem()->indexOfChild(defaults->currentItem());
    if (index < 0) index = 0;
    QTreeWidgetItem *add;

    add = new QTreeWidgetItem;
    defaults->invisibleRootItem()->insertChild(index, add);
    add->setFlags(add->flags() | Qt::ItemIsEditable);

    // field
    QString text = tr("New");
    for (int i=0; defaults->findItems(text, Qt::MatchExactly, 1).count() > 0; i++) {
        text = QString(tr("New (%1)")).arg(i+1);
    }
    add->setText(0, text);
}

void
DefaultsPage::deleteClicked()
{
    if (defaults->currentItem()) {
        int index = defaults->invisibleRootItem()->indexOfChild(defaults->currentItem());

        // zap!
        delete defaults->invisibleRootItem()->takeChild(index);
    }
}

void
DefaultsPage::getDefinitions(QList<DefaultDefinition> &defaultList)
{
    SpecialFields sp;

    // clear current just in case
    defaultList.clear();

    for (int idx =0; idx < defaults->invisibleRootItem()->childCount(); idx++) {

        DefaultDefinition add;
        QTreeWidgetItem *item = defaults->invisibleRootItem()->child(idx);

        add.field = sp.internalName(item->text(0));
        add.value = item->text(1);
        add.linkedField = sp.internalName(item->text(2));
        add.linkedValue = item->text(3);

        defaultList.append(add);
    }
}

IntervalsPage::IntervalsPage(Context *context) : context(context)
{
    // get config
    b4.discovery = appsettings->cvalue(context->athlete->cyclist, GC_DISCOVERY, 57).toInt(); // 57 does not include search for PEAKs

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QGridLayout *layout = new QGridLayout();
    mainLayout->addLayout(layout);
    mainLayout->addStretch();

    QLabel *heading = new QLabel(tr("Enable interval auto-discovery"));
    heading->setFixedHeight(QFontMetrics(heading->font()).height() + 4);

    int row = 0;
    layout->addWidget(heading, row, 0, Qt::AlignRight);

    user = 99;
    for(int i=0; i<= static_cast<int>(RideFileInterval::last()); i++) {

        // ignore until we get past user interval type
        if (i == static_cast<int>(RideFileInterval::USER)) user=i;

        // if past user then add
        if (i>user) {
            QCheckBox *add = new QCheckBox(RideFileInterval::typeDescriptionLong(static_cast<RideFileInterval::IntervalType>(i)));
            checkBoxes << add;
            add->setChecked(b4.discovery & RideFileInterval::intervalTypeBits(static_cast<RideFileInterval::IntervalType>(i)));
            layout->addWidget(add, row++, 1, Qt::AlignLeft);
        }
    }
}

qint32
IntervalsPage::saveClicked()
{
    int discovery = 0;

    // now update discovery !
    for(int i=0; i< checkBoxes.count(); i++)
        if (checkBoxes[i]->isChecked())
            discovery += RideFileInterval::intervalTypeBits(static_cast<RideFileInterval::IntervalType>(i+user+1));

    // new value returned
    appsettings->setCValue(context->athlete->cyclist, GC_DISCOVERY, discovery);

    // return change !
    if (b4.discovery != discovery) return CONFIG_DISCOVERY;
    else return 0;
}
