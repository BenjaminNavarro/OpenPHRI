/*      File: data_logger.cpp
*       This file is part of the program open-phri
*       Program description : OpenPHRI: a generic framework to easily and safely control robots in interactions with humans
*       Copyright (C) 2017 -  Benjamin Navarro (LIRMM). All Right reserved.
*
*       This software is free software: you can redistribute it and/or modify
*       it under the terms of the LGPL license as published by
*       the Free Software Foundation, either version 3
*       of the License, or (at your option) any later version.
*       This software is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*       LGPL License for more details.
*
*       You should have received a copy of the GNU Lesser General Public License version 3 and the
*       General Public License version 3 along with this program.
*       If not, see <http://www.gnu.org/licenses/>.
*/

#include <OpenPHRI/safety_controller.h>
#include <OpenPHRI/utilities/data_logger.h>
#include <OpenPHRI/constraints/constraint.h>
#include <OpenPHRI/force_generators/force_generator.h>
#include <OpenPHRI/torque_generators/torque_generator.h>
#include <OpenPHRI/velocity_generators/velocity_generator.h>
#include <OpenPHRI/joint_velocity_generators/joint_velocity_generator.h>
#include <iomanip>

using namespace phri;


DataLogger::DataLogger(
	const std::string& directory,
	doubleConstPtr time,
	bool create_gnuplot_files,
	bool delay_disk_write) :
	controller_(nullptr),
	directory_(directory),
	time_(time),
	create_gnuplot_files_(create_gnuplot_files),
	delay_disk_write_(delay_disk_write)
{
	if(*directory_.end() != '/') {
		directory_.push_back('/');
	}
}

DataLogger::~DataLogger() {
	writeStoredDataToDisk();
	closeFiles();
}

void DataLogger::logSafetyControllerData(SafetyController* controller) {
	controller_ = controller;
}

void DataLogger::logRobotData(RobotConstPtr robot) {
	size_t joint_vec_size = robot->jointCount();

	logExternalData("jointDampingMatrix", robot->jointDampingMatrix()->data(), joint_vec_size);
	logExternalData("controlPointDampingMatrix", robot->controlPointDampingMatrix()->data(), 6);

	logExternalData("jointVelocity", robot->jointVelocity()->data(), joint_vec_size);
	logExternalData("jointVelocitySum", robot->jointVelocitySum()->data(), joint_vec_size);
	logExternalData("jointTorqueSum", robot->jointTorqueSum()->data(), joint_vec_size);
	logExternalData("jointVelocityCommand", robot->jointVelocityCommand()->data(), joint_vec_size);
	logExternalData("jointTotalVelocity", robot->jointTotalVelocity()->data(), joint_vec_size);
	logExternalData("jointTotalTorque", robot->jointTotalTorque()->data(), joint_vec_size);
	logExternalData("jointCurrentPosition", robot->jointCurrentPosition()->data(), joint_vec_size);
	logExternalData("jointTargetPosition", robot->jointTargetPosition()->data(), joint_vec_size);
	logExternalData("jointExternalTorque", robot->jointExternalTorque()->data(), joint_vec_size);

	logExternalData("controlPointVelocity", robot->controlPointVelocity()->data(), 6);
	logExternalData("controlPointVelocitySum", robot->controlPointVelocitySum()->data(), 6);
	logExternalData("controlPointForceSum", robot->controlPointForceSum()->data(), 6);
	logExternalData("controlPointVelocityCommand", robot->controlPointVelocityCommand()->data(), 6);
	logExternalData("controlPointTotalVelocity", robot->controlPointTotalVelocity()->data(), 6);
	logExternalData("controlPointTotalForce", robot->controlPointTotalForce()->data(), 6);
	logExternalData("controlPointCurrentPosition", robot->controlPointCurrentPose()->translation().data(), 3);
	logExternalData("controlPointCurrentOrientation", robot->controlPointCurrentPose()->orientation().coeffs().data(), 4);
	logExternalData("controlPointTargetPosition", robot->controlPointTargetPose()->translation().data(), 3);
	logExternalData("controlPointTargetOrientation", robot->controlPointTargetPose()->orientation().coeffs().data(), 4);
	logExternalData("controlPointCurrentVelocity", robot->controlPointCurrentVelocity()->data(), 6);
	logExternalData("controlPointExternalForce", robot->controlPointExternalForce()->data(), 6);

	logExternalData("scalingFactor", robot->scalingFactor().get(), 1);

	robot_ = robot;
}

void DataLogger::reset() {
	controller_ = nullptr;
	robot_.reset();
	external_data_.clear();
}

std::ofstream& DataLogger::createLog(const std::string& data_name, size_t data_count) {
	std::string filename = directory_ + "log_" + data_name + ".txt";
	std::transform(filename.begin(), filename.end(), filename.begin(), [](char ch) {
		return ch == ' ' ? '_' : ch;
	});
	auto& file = log_files_[data_name];
	file.open(filename, std::ios_base::trunc);
	if(not file.is_open()) {
		throw std::runtime_error("Unable to open the file " + filename + " for writting");
	}
	file.precision(6);

	if(delay_disk_write_) {
		stored_data_[&file];
	}

	if(create_gnuplot_files_) {
		auto gnuplot_filename = filename;
		gnuplot_filename.resize(filename.size()-4);
		std::ofstream gnuplot_file(gnuplot_filename + ".gnuplot");

		if(gnuplot_file.is_open()) {
			gnuplot_file << "plot ";
			for (int i = 0; i < data_count; ++i) {
				gnuplot_file << "\"" << filename << "\" using 1:" << i+2 << " title '" << data_name << (data_count > 1 ? " " + std::to_string(i+1) : " ") << "' with lines";
				if(i == (data_count-1))
					gnuplot_file << '\n';
				else
					gnuplot_file << ", ";
			}

			gnuplot_file.close();
		}
	}

	return file;
}

std::ostream& DataLogger::getStream(std::ofstream& file) {
	if(delay_disk_write_) {
		return stored_data_.at(&file);
	}
	else {
		return file;
	}
}

void DataLogger::logData(std::ofstream& file, const double* data, size_t data_count) {
	std::ostream& stream = getStream(file);
	stream << *time_ << '\t';
	for (size_t i = 0; i < data_count-1; ++i) {
		stream << data[i] << '\t';
	}
	stream << data[data_count-1] << '\n';
}

void DataLogger::logData(std::ofstream& file, const external_data& data) {
	std::ostream& stream = getStream(file);
	stream << *time_ << '\t';
	data.write(stream);
}

void DataLogger::process() {
	if(controller_ != nullptr) {
		for(auto it=controller_->constraints_begin(); it!=controller_->constraints_end(); ++it) {
			const std::string& data_name = it->first;
			auto& file = log_files_[data_name];
			if(not file.is_open()) {
				createLog(data_name, 1);
			}
			logData(file, &(it->second.last_value), 1);
		}

		for(auto it=controller_->force_generators_begin(); it!=controller_->force_generators_end(); ++it) {
			const std::string& data_name = it->first;
			auto& file = log_files_[data_name];
			if(not file.is_open()) {
				createLog(data_name, 6);
			}
			logData(file, it->second.last_value.data(), 6);
		}

		for(auto it=controller_->torque_generators_begin(); it!=controller_->torque_generators_end(); ++it) {
			const std::string& data_name = it->first;
			auto& file = log_files_[data_name];
			if(not file.is_open()) {
				createLog(data_name, 6);
			}
			logData(file, it->second.last_value.data(), 6);
		}

		for(auto it=controller_->velocity_generators_begin(); it!=controller_->velocity_generators_end(); ++it) {
			const std::string& data_name = it->first;
			auto& file = log_files_[data_name];
			if(not file.is_open()) {
				createLog(data_name, 6);
			}
			logData(file, it->second.last_value.data(), 6);
		}

		for(auto it=controller_->joint_velocity_generators_begin(); it!=controller_->joint_velocity_generators_end(); ++it) {
			const std::string& data_name = it->first;
			auto& file = log_files_[data_name];
			if(not file.is_open()) {
				createLog(data_name, 6);
			}
			logData(file, it->second.last_value.data(), 6);
		}
	}

	for(auto& data: external_data_) {
		logData(*data.first, *data.second);
	}
}

void DataLogger::operator()() {
	process();
}

void DataLogger::writeStoredDataToDisk() {
	for(auto& data: stored_data_) {
		*data.first << data.second.str();
	}
}

void DataLogger::closeFiles() {
	for(auto& file: log_files_) {
		file.second.close();
	}
}
