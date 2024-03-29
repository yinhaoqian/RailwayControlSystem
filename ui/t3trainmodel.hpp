#ifndef T3TRAINMODEL_HPP
#define T3TRAINMODEL_HPP
#include "t3prophelper.h"
class T3TrainModel {
	static bool connectedToTrackModel;
	static bool connectedToTrainController;
	//TRAIN MODEL
  public:

	static void removeTrain(const QString trainId, MODU_ARGS_REF argsref);
	static void createNewTrainFromDispatchRequests(const QList<QJsonObject> *poppedRequests, MODU_ARGS_REF argsref);
	static void createNewTrain(const QString trainId, const QJsonArray path, MODU_ARGS_REF argsref);
	static QJsonArray getAllTrainIds( MODU_ARGS_REF argsref);
	static void setFailureOrBrake(const QString trainId, const int index, const bool value, MODU_ARGS_REF argsref);
	static void setCabinTemperature(const QString trainId, const unsigned int temperature, MODU_ARGS_REF argsref);
	static QJsonArray getStringsFromMetaInfo(const QString trainId, MODU_ARGS_REF argsref);
	static void embarkAndDisembarkPassangerOnAllTrains(MODU_ARGS_REF argsref);
	static void updateTrainVelocityOnAllTrains(float timerRate, MODU_ARGS_REF argsref);
  private:

};

inline void T3TrainModel::removeTrain(const QString trainId, MODU_ARGS_REF argsref) {
	for(qsizetype i = 0; i < std::get<4>(*argsref)->count(); ++i) {
		QJsonObject currTrainObject = std::get<4>(*argsref)->at(i).toObject();
		if(currTrainObject.value(QString("NM_ID")).toString().trimmed() == trainId.trimmed()) {
			std::get<4>(*argsref)->removeAt(i);
			return;
		}
	}
	qFatal("T3Database::removeTrainFromCtc() -> cannot find train id provided.");
}

inline void T3TrainModel::createNewTrainFromDispatchRequests(const QList<QJsonObject> *poppedRequests, MODU_ARGS_REF argsref) {
	for(const QJsonObject& currRequest : *poppedRequests) {
		QString trainId = currRequest.value("trainId").toString();
		QJsonArray path = currRequest.value("path").toArray();
		//determine if the train is moving forward or backward
		createNewTrain(trainId, path, argsref);
		Q_ASSERT(path.size() > 0);
		//SET_TRAIN_F(trainId, "NM_BLOCKID", path.at(0).toString(), argsref);
	}
}


inline void T3TrainModel::createNewTrain(const QString trainId, const QJsonArray path, MODU_ARGS_REF argsref) {
	QJsonObject trainObject;
	trainObject.insert(QString("NM_ID"), trainId);
	trainObject.insert(QString("NM_LENGTH"), 32.2);
	trainObject.insert(QString("NM_HEIGHT"), 3.42);
	trainObject.insert(QString("NM_WIDTH"), 2.65);
	trainObject.insert(QString("NM_MASS"), 40.9e3);
	trainObject.insert(QString("NM_ACCELERATION"), 0.0);
	//trainObject.insert(QString("velocity"), 0.0);
	trainObject.insert(QString("NM_CREWCOUNT"), 1);
	trainObject.insert(QString("NM_PASSANGERCOUNT"), 0);
	trainObject.insert(QString("NM_TEMPERATURE"), 68.0f);
	trainObject.insert(QString("NM_AUTOMODE"), true);
	trainObject.insert(QString("NM_HEATER"), false);
	trainObject.insert(QString("COM[NC_NM]_EXTLIGHT"), false);
	trainObject.insert(QString("COM[NC_NM]_INTLIGHT"), false);
	trainObject.insert(QString("COM[NC_NM]_LDOOR"), false);
	trainObject.insert(QString("COM[NC_NM]_RDOOR"), false);
	trainObject.insert(QString("COM[NC_NM]_EBRAKE"), false);
	trainObject.insert(QString("COM[NC_NM]_SBRAKE"), false);
	trainObject.insert(QString("COM[NC_NM]_FAILURE0"), false);
	trainObject.insert(QString("COM[NC_NM]_FAILURE1"), false);
	trainObject.insert(QString("COM[NC_NM]_FAILURE2"), false);
	trainObject.insert(QString("COM[NC_NM]_SECLEFT"), -1);//output power
	trainObject.insert(QString("NC_PREVE"), 0.0);//error
	trainObject.insert(QString("NC_PREVY"), 0.0);//velocity
	trainObject.insert(QString("NC_SUME"), 0.0);
	trainObject.insert(QString("NC_R"), 0.0);//velocity set point
	trainObject.insert(QString("NC_KI"), 0.5);//
	trainObject.insert(QString("NC_KP"), 0.5);
	trainObject.insert(QString("NC_U"), 0.0);//output power

	Q_ASSERT(path.size() >= 1);
	trainObject.insert(QString("NM_BLOCKID"), path.at(0).toString());
	//trainObject.insert(QString("NM_NEXTBLOCKID"), path.at(1).toString());
	std::get<4>(*argsref)->push_front(trainObject);
}




inline QJsonArray T3TrainModel::getAllTrainIds(MODU_ARGS_REF argsref) {
	QJsonArray toRet;
	QStringList allTrainIds;
	for(const QJsonValue& currTrainObject : qAsConst(*std::get<4>(*argsref))) {
		Q_ASSERT(currTrainObject.isObject() && currTrainObject.toObject().contains("NM_ID"));
		allTrainIds.push_back(currTrainObject.toObject().value("NM_ID").toString());
	}
	toRet.push_back(QJsonArray::fromStringList(allTrainIds));
	return toRet;
}

inline void T3TrainModel::setFailureOrBrake(const QString trainId, const int index, const bool value, MODU_ARGS_REF argsref) {
	switch (index) {
	case 0:
		SET_TRAIN_F(trainId, "COM[NC_NM]_FAILURE0", value, argsref);
		break;
	case 1:
		SET_TRAIN_F(trainId, "COM[NC_NM]_FAILURE1", value, argsref);
		break;
	case 2:
		SET_TRAIN_F(trainId, "COM[NC_NM]_FAILURE2", value, argsref);
		break;
	case 3:
		SET_TRAIN_F(trainId, "COM[NC_NM]_EBRAKE", value, argsref);
		break;
	default:
		qFatal("T3TrainModel::setFailureOrBrake() -> index not recognized");
		break;
	}
	return;
}

inline void T3TrainModel::setCabinTemperature(const QString trainId, const unsigned int temperature, MODU_ARGS_REF argsref) {
	SET_TRAIN_F(trainId, "NM_TEMPERATURE", temperature, argsref);
}

inline QJsonArray T3TrainModel::getStringsFromMetaInfo(const QString trainId, MODU_ARGS_REF argsref) {
	QVariantList metaInfo;
	{
		//0 - left door
		bool leftDoorOpened = GET_TRAIN_F(trainId, "COM[NC_NM]_LDOOR", argsref).toBool();
		metaInfo.append(leftDoorOpened ? "OPENED" : "CLOSED");
	}
	{
		//1- right door
		bool rightDoorOpened = GET_TRAIN_F(trainId, "COM[NC_NM]_RDOOR", argsref).toBool();
		metaInfo.append(rightDoorOpened ? "OPENED" : "CLOSED");
	}
	{
		//2- service brake
		bool sBrake = GET_TRAIN_F(trainId, "COM[NC_NM]_SBRAKE", argsref).toBool();
		metaInfo.append(sBrake ? "APPLIED" : "RELEASED");
	}
	{
		//3,4 - emergency brake
		bool eBrake = GET_TRAIN_F(trainId, "COM[NC_NM]_EBRAKE", argsref).toBool();
		metaInfo.append(eBrake ? "APPLIED" : "RELEASED");
		metaInfo.append(eBrake);
	}

	const QString blockId = GET_TRAIN_F(trainId, "NM_BLOCKID", argsref).toString();
	if(blockId == "")
		qFatal("T3TrainController::getMetaInfo() -> BLOCK ID is empty.");
	const QString BCNPLCOUT = GET_TRACKVAR_F(blockId, "COM[KC|KM]_BCNPLCOUT", argsref).toString();
	//metaInfo.append(0);//unused
	{
		//5- speed limit
		unsigned int speedLimit =  KMH2MPH_F(GET_TRACKCON_F(blockId, "speedLimit", argsref).toUInt());
		metaInfo.append(speedLimit);
	}

	{
		//6- light signal
		unsigned int authorizedNumberBlock =  BCNPLCOUT.midRef(2, 8).toUInt(nullptr, 2);
		if(authorizedNumberBlock > 3) metaInfo.append(QString("green"));
		else if(authorizedNumberBlock > 1) metaInfo.append(QString("yellow"));
		else metaInfo.append(QString("red"));
	}
	{
		//7- powert
		float power  = GET_TRAIN_F(trainId, "NC_U", argsref).toDouble();
		metaInfo.append(static_cast<int>(power));
	}
	{
		//8- imperial velocity
		float impVelocity  = KMH2MPH_F(GET_TRAIN_F(trainId, "NC_PREVY", argsref).toFloat());
		metaInfo.append(static_cast<int>(impVelocity));
	}
	{
		//9- imperial acceleration
		float impAccel  = M2FOOT_F(GET_TRAIN_F(trainId, "NM_ACCELERATION", argsref).toFloat());
		metaInfo.append(static_cast<int>(impAccel));
	}
	{
		//10-length string
		float length = M2FOOT_F(GET_TRAIN_F(trainId, "NM_LENGTH", argsref).toFloat());
		metaInfo.append(QString::number(length, 'g', 3) + " FT");
	}
	{
		//11 height string
		float height = M2FOOT_F(GET_TRAIN_F(trainId, "NM_HEIGHT", argsref).toFloat());
		metaInfo.append(QString::number(height, 'g', 3) + " FT");
	}
	{
		//12-width string
		float width = M2FOOT_F(GET_TRAIN_F(trainId, "NM_WIDTH", argsref).toFloat());
		metaInfo.append(QString::number(width, 'g', 3) + " FT");
	}
	{
		//13-mass string
		float mass = (GET_TRAIN_F(trainId, "NM_MASS", argsref).toFloat());
		metaInfo.append(QString::number(mass * 0.001, 'g', 3) + " T");
	}
	{
		//14,- heater || 15-unused
		bool heater = (GET_TRAIN_F(trainId, "NM_HEATER", argsref).toFloat());
		metaInfo.append(QString(heater ? "ON" : "OFF"));
		metaInfo.append(0);
	}
	{
		//16-ext light string
		bool extLight = (GET_TRAIN_F(trainId, "COM[NC_NM]_EXTLIGHT", argsref).toBool());
		metaInfo.append(QString(extLight ? "ON" : "OFF"));
	}
	{
		//17-int light string
		bool intLight = (GET_TRAIN_F(trainId, "COM[NC_NM]_INTLIGHT", argsref).toBool());
		metaInfo.append(QString(intLight ? "ON" : "OFF"));
	}
	{
		//18,19-failure 1 string
		bool f1 = (GET_TRAIN_F(trainId, "COM[NC_NM]_FAILURE0", argsref).toBool());
		metaInfo.append(QString(f1 ? "ERROR" : "OK"));
		metaInfo.append(f1);
	}
	{
		//20,21 failure 2 string
		bool f2 = (GET_TRAIN_F(trainId, "COM[NC_NM]_FAILURE1", argsref).toBool());
		metaInfo.append(QString(f2 ? "ERROR" : "OK"));
		metaInfo.append(f2);
	}
	{
		//22,23-failure 3 string
		bool f3 = (GET_TRAIN_F(trainId, "COM[NC_NM]_FAILURE2", argsref).toBool());
		metaInfo.append(QString(f3 ? "ERROR" : "OK"));
		metaInfo.append(f3);
	}
	{
		//24,25 - train cabin temperature in F
		unsigned int temperature = GET_TRAIN_F(trainId, "NM_TEMPERATURE", argsref).toUInt();
		metaInfo.append(QString::number(temperature) + " F");
		metaInfo.append(temperature);
	}
	{
		//26 - crew count
		unsigned int crewCount = GET_TRAIN_F(trainId, "NM_CREWCOUNT", argsref).toUInt();
		metaInfo.append(QString::number(crewCount));
	}
	{
		//27 - passanger count
		unsigned int passangerCount = GET_TRAIN_F(trainId, "NM_PASSANGERCOUNT", argsref).toUInt();
		metaInfo.append(QString::number(passangerCount));
	}
	uint authCount = BCNPLCOUT.midRef(2, 8).toUInt(nullptr, 2);
	{
		//28- authority granted
		metaInfo.append(QString(authCount > 0 ? "YES" : "NO"));
	}
	{
		//29 - authority block numbers
		metaInfo.append(QString::number(authCount) + " BLOCKS");
	}
	{
		int secondsLeft = GET_TRAIN_F(trainId, "COM[NC_NM]_SECLEFT", argsref).toInt();
		//30 - station string
		if(BCNPLCOUT[26] == '1' || BCNPLCOUT[27] == '1') {
			QString toDisp;
			toDisp += (BCNPLCOUT[26] == '1') ? "PLATFORM" : "--------";
			toDisp += " | TRAIN | ";
			toDisp += (BCNPLCOUT[27] == '1') ? "PLATFORM" : "--------";
			if(secondsLeft == 20) {
				toDisp += "\nTRAIN IS ENTERING STATION";
			} else if(secondsLeft == -2) {
				toDisp += "\nTRAIN IS LEAVING STATION";
			} else {
				toDisp += QString("\nTRAIN WILL LEAVE STATION IN %1 SECONDS").arg(qMax(secondsLeft - 5, 0));
			}
			metaInfo.push_back(toDisp);
		} else {
			metaInfo.push_back(QString(""));
		}
	}
	{
		//31 - bool -> if on switch
		bool isOnSwitchBlock =
			GET_TRACKCON_F(blockId, "nextBlock2", argsref).toString() != ""
			|| GET_TRACKCON_F(blockId, "prevBlock2", argsref).toString() != "";
		metaInfo.push_back(isOnSwitchBlock);
	}
	return QJsonArray::fromVariantList(metaInfo);
}

inline void T3TrainModel::embarkAndDisembarkPassangerOnAllTrains(MODU_ARGS_REF argsref) {
	for(const QJsonValue& currTrainRaw : qAsConst(*std::get<4>(*argsref))) {
		QJsonObject currTrain = currTrainRaw.toObject();
		QString trainId = currTrain.value("NM_ID").toString();
		QString blockId = currTrain.value("NM_BLOCKID").toString();
		QString BCNPLCOUT = GET_TRACKVAR_F(blockId, "COM[KC|KM]_BCNPLCOUT", argsref).toString();
		if(!(BCNPLCOUT.at(26) == '1' || BCNPLCOUT.at(27) == '1')) continue;
		int secondsLeft = currTrain.value("COM[NC_NM]_SECLEFT").toInt();
		if(secondsLeft == 25) {
			uint passangerCount = currTrain.value("NM_PASSANGERCOUNT").toInt();
			uint peopleOnStation = GET_TRACKVAR_F(blockId, "KM_PEOPLEONSTATION", argsref).toUInt();
			uint fromTrainToStation = QRandomGenerator::global()->bounded(0u, passangerCount);
			uint fromStationToTrain = QRandomGenerator::global()->bounded(0u, peopleOnStation);
			SET_TRACKVAR_F(blockId, "KM_TRAINTOSTATIONCOUNT", fromTrainToStation, argsref);
			SET_TRACKVAR_F(blockId, "KM_STATIONTOTRAINCOUNT", fromStationToTrain, argsref);
			passangerCount =  passangerCount - fromTrainToStation + fromStationToTrain;
			peopleOnStation = peopleOnStation + fromTrainToStation - fromStationToTrain;
			SET_TRACKVAR_F(blockId, "KM_PEOPLEONSTATION", peopleOnStation, argsref);
			SET_TRAIN_F(trainId, "NM_PASSANGERCOUNT", passangerCount, argsref);
		}
	}
}

inline void T3TrainModel::updateTrainVelocityOnAllTrains(float timerRate, MODU_ARGS_REF argsref) {
	for(const QJsonValue& currTrainRaw : qAsConst(*std::get<4>(*argsref))) {
		QJsonObject currTrain = currTrainRaw.toObject();
		//Oh dear here comes the physics
		QString blockId = currTrain.value("NM_BLOCKID").toString();
		const float dt = static_cast<uint>(timerRate);
		const float g = 9.8f;
		const float mul = 5e2f;
		float grade = GET_TRACKCON_F(blockId, "grade", argsref).toDouble();
		float velocity = KMH2MS_F(currTrain.value("NC_PREVY").toDouble());
		float accel = currTrain.value("NM_ACCELERATION").toDouble();
		float mass = currTrain.value("NM_MASS").toDouble();
		float power = currTrain.value("NC_U").toDouble();
		float theta = qAtan(grade / 100);
		float F_grade = mass * g * qSin(theta);
		float F_engine = mul * power / velocity;
		//qDebug() << QString("POWER: %1 , VELOC: %2, FGRADE %3, F ENGINE %4").arg(power).arg(velocity).arg(F_grade).arg(F_engine);
		float accel_new = (F_engine - F_grade) / mass;
		if(qIsInf(accel_new)) accel_new = 0.1f;
		qDebug() << QString("F_grade = %1 , F_engine = %2 , Accel = %3").arg(F_grade).arg(F_engine).arg(accel_new);
		if(currTrain.value("COM[NC_NM]_EBRAKE").toBool() && velocity > 0) {
			accel_new = -2.73 ;
		} else if(currTrain.value("COM[NC_NM]_SBRAKE").toBool() && velocity > 0) {
			accel_new = -0.12 * 5 ;
		}
		//qDebug() << QString("accel: %1").arg(accel_new);
		//brakes
//		if(currTrain.value("COM[NC_NM]_EBRAKE").toBool()) {
//			accel_new = -1.2f;
//		} else if(currTrain.value("COM[NC_NM]_SBRAKE").toBool()) {
//			accel_new = -2.73f;
//		}
		//maximum and minimum speed

		float velocity_new = velocity + dt * (accel_new + accel) / 2;

//		if(currTrain.value("COM[NC_NM]_EBRAKE").toBool() || currTrain.value("COM[NC_NM]_SBRAKE").toBool()) {
//			if(velocity_new < - 3) velocity_new += 1;
//			else if(velocity_new > 3) velocity_new -= 1;
//			else if(velocity_new == 0)velocity_new = 0;
//			accel_new = 0;
//		} else {
//			velocity_new =   + dt * (accel_new + accel) / 2;
//		}
		//if(velocity_new < 0) velocity_new = 0;
		//qDebug() << QString("-1- %1 + %2 * ( %3 + %4) = %5").arg(velocity).arg(dt).arg(accel_new).arg(accel).arg(velocity_new);
		if(velocity_new - velocity > 1) {
			velocity_new = velocity + 1;
		} else if(velocity_new > 19.44f) {
			velocity_new = 19.44f;
//			if(accel_new > 0) accel_new = 0;
		} else if(velocity_new < 0) {
			velocity_new = 0.0f;
//			if(accel_new < 0) accel_new = 0;
		}
		SET_TRAIN_F(currTrain.value("NM_ID").toString(), "NM_ACCELERATION", accel_new, argsref);
		SET_TRAIN_F(currTrain.value("NM_ID").toString(), "NC_PREVY", MS2KMH_F(velocity_new), argsref);
	}
}





#endif // T3TrainModel_HPP
