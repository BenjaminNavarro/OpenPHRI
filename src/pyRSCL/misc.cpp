#include <RSCL/RSCL.h>
#include "double_wrappers.h"

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

void assert_msg_py(std::string msg, bool cond) {
	if(not cond) {
		std::cout << "Failed: " << msg << std::endl;
		exit(-1);
	}
}

std::shared_ptr<Matrix6d> NewMatrix6dPtr(Matrix6d init_value = Matrix6d::Zero())
{
	return std::make_shared<Matrix6d>(init_value);
}
BOOST_PYTHON_FUNCTION_OVERLOADS(NewMatrix6dPtr_overloads, NewMatrix6dPtr, 0, 1)

std::shared_ptr<Vector2d> NewVector2dPtr(Vector2d init_value = Vector2d::Zero())
{
	return std::make_shared<Vector2d>(init_value);
}
BOOST_PYTHON_FUNCTION_OVERLOADS(NewVector2dPtr_overloads, NewVector2dPtr, 0, 1)

std::shared_ptr<Vector3d> NewVector3dPtr(Vector3d init_value = Vector3d::Zero())
{
	return std::make_shared<Vector3d>(init_value);
}
BOOST_PYTHON_FUNCTION_OVERLOADS(NewVector3dPtr_overloads, NewVector3dPtr, 0, 1)

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


} // namespace RSCL

void wrapMisc() {
	using namespace RSCL;
	using namespace boost::python;

	def("assert_msg",                &assert_msg_py,           "Call assert and print a message if it failts");

	def("NewMatrix6dPtr",
	    NewMatrix6dPtr,
	    NewMatrix6dPtr_overloads(
			args("init_value"), "Create a new instance of a Matrix6d shared_ptr"));

	def("NewVector2dPtr",
	    NewVector2dPtr,
	    NewVector2dPtr_overloads(
			args("init_value"), "Create a new instance of a Vector2d shared_ptr"));

	def("NewVector3dPtr",
	    NewVector3dPtr,
	    NewVector3dPtr_overloads(
			args("init_value"), "Create a new instance of a Vector3d shared_ptr"));

	def("NewVector6dPtr",
	    NewVector6dPtr,
	    NewVector6dPtr_overloads(
			args("init_value"), "Create a new instance of a Vector6d shared_ptr"));

	def("NewDoublePtr",
	    NewDoublePtr,
	    NewDoublePtr_overloads(
			args("init_value"), "Create a new instance of a double shared_ptr"));

	register_ptr_to_python<std::shared_ptr<Matrix6d>>();
	register_ptr_to_python<std::shared_ptr<Vector2d>>();
	register_ptr_to_python<std::shared_ptr<Vector3d>>();
	register_ptr_to_python<std::shared_ptr<Vector6d>>();
	register_ptr_to_python<std::shared_ptr<DoubleWrap>>();
	register_ptr_to_python<std::shared_ptr<const Matrix6d>>();
	register_ptr_to_python<std::shared_ptr<const Vector2d>>();
	register_ptr_to_python<std::shared_ptr<const Vector3d>>();
	register_ptr_to_python<std::shared_ptr<const Vector6d>>();
	register_ptr_to_python<std::shared_ptr<ConstDoubleWrap>>();

	implicitly_convertible<std::shared_ptr<Matrix6d>, std::shared_ptr<const Matrix6d>>();
	implicitly_convertible<std::shared_ptr<Vector2d>, std::shared_ptr<const Vector2d>>();
	implicitly_convertible<std::shared_ptr<Vector3d>, std::shared_ptr<const Vector3d>>();
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

}
