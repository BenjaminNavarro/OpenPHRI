#include <RSCL/RSCL.h>
#include <vrep_driver/vrep_driver.h>

#include <boost/python.hpp>
#include <iostream>

namespace boost {

// Make boost::python understand std::shared_ptr
template <class T>
T* get_pointer(std::shared_ptr<T> p)
{
	return p.get();
}

} // namespace boost

namespace RSCL {

using namespace vrep;

void assert_msg_py(std::string msg, bool cond) {
	if(not cond) {
		std::cout << "Failed: " << msg << std::endl;
		exit(-1);
	}
}

// Primitive types need to be wrapped in order to use pointers on them in python
struct DoubleWrap {
	DoubleWrap(double init_value = 0.) :
		value(std::make_shared<double>(init_value))
	{
	}

	DoubleWrap(std::shared_ptr<double> ptr) :
		value(ptr)
	{
	}

	operator std::shared_ptr<double>() const {
		return value;
	}

	operator std::shared_ptr<const double>() const {
		return value;
	}

	void set(double val) {
		*value = val;
	}

	double get() const {
		return *value;
	}

private:
	std::shared_ptr<double> value;
};

struct ConstDoubleWrap {
	ConstDoubleWrap(std::shared_ptr<const double> ptr) :
		value(ptr.get())
	{
	}

	ConstDoubleWrap(const double* ptr) :
		value(ptr)
	{
	}

	operator std::shared_ptr<const double>() const {
		return std::shared_ptr<const double>(value);
	}

	ConstDoubleWrap operator=(const double* ptr) {
		value = ptr;
		return *this;
	}

	double get() const {
		return *value;
	}

private:
	const double* value;
};

// Wrappers for classes returning pointers to double
struct PowerConstraintWrap : public PowerConstraint {
	PowerConstraintWrap(
		Vector6dConstPtr total_velocity,
		Vector6dConstPtr external_force,
		doubleConstPtr maximum_power) :
		PowerConstraint(total_velocity, external_force, maximum_power)
	{
	}

	ConstDoubleWrap getPowerPy() {
		return ConstDoubleWrap(getPower());
	}
};


// Functions to create shared pointers to constraints
std::shared_ptr<DefaultConstraint> NewDefaultConstraint(ConstraintType type)
{
	return std::make_shared<DefaultConstraint>(type);
}

std::shared_ptr<VelocityConstraint> NewVelocityConstraint(Vector6dPtr total_velocity, doublePtr maximum_velocity)
{
	return std::make_shared<VelocityConstraint>(static_cast<Vector6dConstPtr>(total_velocity), static_cast<doublePtr>(maximum_velocity));
}

std::shared_ptr<PowerConstraintWrap> NewPowerConstraint(Vector6dPtr total_velocity, Vector6dPtr external_force, doublePtr maximum_power)
{
	return std::make_shared<PowerConstraintWrap>(static_cast<Vector6dConstPtr>(total_velocity), static_cast<Vector6dConstPtr>(external_force), static_cast<doublePtr>(maximum_power));
}

std::shared_ptr<StopConstraint> NewStopConstraint(Vector6dPtr external_force, doublePtr activation_force_threshold, doublePtr deactivation_force_threshold)
{
	return std::make_shared<StopConstraint>(static_cast<Vector6dConstPtr>(external_force), static_cast<doublePtr>(activation_force_threshold), static_cast<doublePtr>(deactivation_force_threshold));
}

std::shared_ptr<VelocityProxy> NewVelocityProxy(Vector6dPtr velocity)
{
	return std::make_shared<VelocityProxy>(static_cast<Vector6dConstPtr>(velocity));
}

std::shared_ptr<ForceProxy> NewForceProxy(Vector6dPtr force)
{
	return std::make_shared<ForceProxy>(static_cast<Vector6dConstPtr>(force));
}

std::shared_ptr<SafetyController> NewSafetyController(Matrix6dPtr damping_matrix)
{
	return std::make_shared<SafetyController>(static_cast<Matrix6dConstPtr>(damping_matrix));
}

std::shared_ptr<VREPDriver> NewVREPDriver(double sample_time, const std::string& prefix = "", const std::string& suffix = "", const std::string& ip = "127.0.0.1", int port = 19997)
{
	return std::make_shared<VREPDriver>(sample_time, prefix, suffix, ip, port);
}
BOOST_PYTHON_FUNCTION_OVERLOADS(NewVREPDriver_overloads, NewVREPDriver, 1, 5)

std::shared_ptr<Matrix6d> NewMatrix6dPtr(Matrix6d init_value = Matrix6d::Zero())
{
	return std::make_shared<Matrix6d>(init_value);
}
BOOST_PYTHON_FUNCTION_OVERLOADS(NewMatrix6dPtr_overloads, NewMatrix6dPtr, 0, 1)

std::shared_ptr<Vector6d> NewVector6dPtr(Vector6d init_value = Vector6d::Zero())
{
	return std::make_shared<Vector6d>(init_value);
}
BOOST_PYTHON_FUNCTION_OVERLOADS(NewVector6dPtr_overloads, NewVector6dPtr, 0, 1)


std::shared_ptr<DoubleWrap> NewDoublePtr(double init_value = 0.)
{
	return std::make_shared<DoubleWrap>(init_value);
}
BOOST_PYTHON_FUNCTION_OVERLOADS(NewDoublePtr_overloads, NewDoublePtr, 0, 1)


// Safety controller methods overloads
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(SafetyController_addConstraint_overloads,        addConstraint,           2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(SafetyController_addVelocityGenerator_overloads, addVelocityGenerator,    2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(SafetyController_addForceGenerator_overloads,    addForceGenerator,       2, 3)


} // namespace RSCL

BOOST_PYTHON_MODULE(PyRSCL) {
	using namespace RSCL;

	using namespace vrep;
	using namespace boost::python;

	def("assert_msg",                &assert_msg_py,           "Call assert and print a message if it failts");

	def("NewMatrix6dPtr",
	    NewMatrix6dPtr,
	    NewMatrix6dPtr_overloads(
			args("init_value"), "Create a new instance of a Matrix6d shared_ptr"));

	def("NewVector6dPtr",
	    NewVector6dPtr,
	    NewVector6dPtr_overloads(
			args("init_value"), "Create a new instance of a Vector6d shared_ptr"));

	def("NewDoublePtr",
	    NewDoublePtr,
	    NewDoublePtr_overloads(
			args("init_value"), "Create a new instance of a double shared_ptr"));

	register_ptr_to_python<std::shared_ptr<Matrix6d>>();
	register_ptr_to_python<std::shared_ptr<Vector6d>>();
	register_ptr_to_python<std::shared_ptr<DoubleWrap>>();
	register_ptr_to_python<std::shared_ptr<const Matrix6d>>();
	register_ptr_to_python<std::shared_ptr<const Vector6d>>();
	register_ptr_to_python<std::shared_ptr<ConstDoubleWrap>>();

	implicitly_convertible<std::shared_ptr<Matrix6d>, std::shared_ptr<const Matrix6d>>();
	implicitly_convertible<std::shared_ptr<Vector6d>, std::shared_ptr<const Vector6d>>();
	implicitly_convertible<std::shared_ptr<double>, std::shared_ptr<const double>>();
	implicitly_convertible<DoubleWrap, std::shared_ptr<double>>();
	implicitly_convertible<std::shared_ptr<double>, DoubleWrap>();
	implicitly_convertible<ConstDoubleWrap, std::shared_ptr<const double>>();
	implicitly_convertible<std::shared_ptr<const double>, ConstDoubleWrap>();

	class_<DoubleWrap, boost::noncopyable>("DoubleWrap")
	.def("set", &DoubleWrap::set, "Set the internal value")
	.def("get", &DoubleWrap::get, "Get the internal value");

	class_<ConstDoubleWrap>("ConstDoubleWrap", init<doubleConstPtr>())
	.def("get", &ConstDoubleWrap::get, "Get the internal value");


	/**********************************************************************************/
	/*                                 Constraint bindings                            */
	/**********************************************************************************/
	def("NewDefaultConstraint",            &NewDefaultConstraint,          "Create a new instance of a DefaultConstraint shared_ptr");
	def("NewVelocityConstraint",           &NewVelocityConstraint,         "Create a new instance of a VelocityConstraint shared_ptr");
	def("NewPowerConstraint",              &NewPowerConstraint,            "Create a new instance of a PowerConstraint shared_ptr");
	def("NewStopConstraint",               &NewStopConstraint,             "Create a new instance of a StopConstraint shared_ptr");

	enum_<ConstraintType>("ConstraintsType", "Type of constraint")
	.value("Multiplicative", ConstraintType::Multiplicative)
	.value("Minimum", ConstraintType::Minimum);

	struct ConstraintWrap : Constraint, wrapper<Constraint> {
		using Constraint::Constraint;

		double compute()
		{
			return this->get_override("compute")();
		}
	};

	class_<ConstraintWrap, boost::noncopyable>("Constraint", init<ConstraintType>())
	.def("getType", &Constraint::getType)
	.def("compute", pure_virtual(&Constraint::compute));

	class_<DefaultConstraint, boost::noncopyable, bases<Constraint>>("DefaultConstraint", "Default constraint, always evaluates to 1", init<ConstraintType>())
	.def("compute", &DefaultConstraint::compute);

	class_<VelocityConstraint, boost::noncopyable, bases<Constraint>>("VelocityConstraint", "Velocity constraint, limits the tool's maximum velocity", init<Vector6dConstPtr, doubleConstPtr>())
	.def("compute", &VelocityConstraint::compute);

	class_<PowerConstraintWrap, boost::noncopyable, bases<Constraint>>("PowerConstraint", "Power constraint, limits the exchanged power between the tool and the environment", init<Vector6dConstPtr, Vector6dConstPtr, doubleConstPtr>())
	.def("compute", &PowerConstraintWrap::compute)
	.def("getPower", &PowerConstraintWrap::getPowerPy);

	class_<StopConstraint, boost::noncopyable, bases<Constraint>>("StopConstraint", "Stop constraint, stops the tool when the external force is above a given limit", init<Vector6dConstPtr, doubleConstPtr, doubleConstPtr>())
	.def("compute", &StopConstraint::compute);

	register_ptr_to_python<std::shared_ptr<DefaultConstraint>>();
	register_ptr_to_python<std::shared_ptr<VelocityConstraint>>();
	register_ptr_to_python<std::shared_ptr<PowerConstraintWrap>>();
	register_ptr_to_python<std::shared_ptr<StopConstraint>>();

	implicitly_convertible<std::shared_ptr<DefaultConstraint>,     std::shared_ptr<Constraint>>();
	implicitly_convertible<std::shared_ptr<VelocityConstraint>,    std::shared_ptr<Constraint>>();
	implicitly_convertible<std::shared_ptr<PowerConstraintWrap>,   std::shared_ptr<Constraint>>();
	implicitly_convertible<std::shared_ptr<StopConstraint>,        std::shared_ptr<Constraint>>();

	/**********************************************************************************/
	/*                                 Generators bindings                            */
	/**********************************************************************************/
	def("NewVelocityProxy",    &NewVelocityProxy,  "Create a new instance of a VelocityProxy shared_ptr");
	def("NewForceProxy",       &NewForceProxy,     "Create a new instance of a ForceProxy shared_ptr");

	struct VelocityGeneratorWrap : VelocityGenerator, wrapper<VelocityGenerator> {
		using VelocityGenerator::VelocityGenerator;

		Vector6d compute()
		{
			return this->get_override("compute")();
		}
	};

	class_<VelocityGeneratorWrap, boost::noncopyable>("VelocityGenerator")
	.def("compute", pure_virtual(&VelocityGenerator::compute));

	class_<VelocityProxy, boost::noncopyable>("VelocityProxy", init<Vector6dConstPtr>())
	.def("compute", pure_virtual(&VelocityProxy::compute));


	struct ForceGeneratorWrap : ForceGenerator, wrapper<ForceGenerator> {
		using ForceGenerator::ForceGenerator;

		Vector6d compute()
		{
			return this->get_override("compute")();
		}
	};

	class_<ForceGeneratorWrap, boost::noncopyable>("ForceGenerator")
	.def("compute", pure_virtual(&ForceGenerator::compute));

	class_<ForceProxy, boost::noncopyable>("ForceProxy", init<Vector6dConstPtr>())
	.def("compute", pure_virtual(&ForceProxy::compute));

	register_ptr_to_python<std::shared_ptr<VelocityProxy>>();
	register_ptr_to_python<std::shared_ptr<ForceProxy>>();

	implicitly_convertible<std::shared_ptr<VelocityProxy>, std::shared_ptr<VelocityGenerator>>();
	implicitly_convertible<std::shared_ptr<ForceProxy>,    std::shared_ptr<ForceGenerator>>();

	/*********************************************************************************/
	/*                          SafetyController bindings                            */
	/*********************************************************************************/

	def("NewSafetyController",       &NewSafetyController,     "Create a new instance of a SafetyController shared_ptr");

	class_<SafetyController, boost::noncopyable>("SafetyController", "Generates a tool velocity based on force and velocity inputs and a set of constraints", init<Matrix6dConstPtr>())
	.def("setVerbose",              &SafetyController::setVerbose)
	.def("addConstraint",           &SafetyController::addConstraint,           SafetyController_addConstraint_overloads(       args("name", "constraint", "force")))
	.def("addForceGenerator",       &SafetyController::addForceGenerator,       SafetyController_addForceGenerator_overloads(   args("name", "generator", "force")))
	.def("addVelocityGenerator",    &SafetyController::addVelocityGenerator,    SafetyController_addVelocityGenerator_overloads(args("name", "generator", "force")))
	.def("removeConstraint",        &SafetyController::removeConstraint)
	.def("removeForceGenerator",    &SafetyController::removeForceGenerator)
	.def("removeVelocityGenerator", &SafetyController::removeVelocityGenerator)
	.def("getConstraint",           &SafetyController::getConstraint)
	.def("getForceGenerator",       &SafetyController::getForceGenerator)
	.def("getVelocityGenerator",    &SafetyController::getVelocityGenerator)
	.def("updateTCPVelocity",       &SafetyController::updateTCPVelocity)
	.def("getTCPVelocity",          &SafetyController::getTCPVelocity)
	.def("getTotalVelocity",        &SafetyController::getTotalVelocity)
	.def("getTotalForce",           &SafetyController::getTotalForce);

	register_ptr_to_python<std::shared_ptr<SafetyController>>();

	/*********************************************************************************/
	/*                               VREPDriver bindings                             */
	/*********************************************************************************/
	enum_<ReferenceFrame>("ReferenceFrame", "Frame of reference")
	.value("TCP", ReferenceFrame::TCP)
	.value("Base", ReferenceFrame::Base)
	.value("World", ReferenceFrame::World);

	def("NewVREPDriver",
	    NewVREPDriver,
	    NewVREPDriver_overloads(
			args("sample_time", "prefix", "suffix", "ip", "port"), "Create a new instance of a VREPDriver shared_ptr"));

	class_<VREPDriver, boost::noncopyable>("VREPDriver", "Interface to the V-REP simulator", init<double, const std::string&, const std::string&, const std::string&, int>())
	.def(init<double, int, const std::string&, const std::string&>())
	.def("getClientID",           &VREPDriver::getClientID)
	.def("checkConnection",       &VREPDriver::checkConnection)
	.def("enableSynchonous",      &VREPDriver::enableSynchonous)
	.def("nextStep",              &VREPDriver::nextStep)
	.def("startSimulation",       &VREPDriver::startSimulation)
	.def("stopSimulation",        &VREPDriver::stopSimulation)
	.def("pauseSimulation",       &VREPDriver::pauseSimulation)
	.def("readTCPPose",           &VREPDriver::readTCPPose)
	.def("readTCPVelocity",       &VREPDriver::readTCPVelocity)
	.def("readTCPTargetPose",     &VREPDriver::readTCPTargetPose)
	.def("sendTCPtargetVelocity", &VREPDriver::sendTCPtargetVelocity)
	.def("readTCPWrench",         &VREPDriver::readTCPWrench);

	register_ptr_to_python<std::shared_ptr<VREPDriver>>();
}
