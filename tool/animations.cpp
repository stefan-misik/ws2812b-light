#include <cstdint>
#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "app/animation.hpp"
#include "app/animation_storage.hpp"
#include "led_strip.hpp"


namespace py = pybind11;


PYBIND11_MODULE(animations, m)
{
    py::class_<LedState>(m, "LedState")
        .def(py::init<>())
        .def(py::init<std::uint8_t, std::uint8_t, std::uint8_t>())
        .def(py::init<std::uint32_t>())
        .def_readwrite("red", &LedState::red)
        .def_readwrite("green", &LedState::green)
        .def_readwrite("blue", &LedState::blue);

    py::class_<Animation>(m, "Animation")
        .def("getParameter", &Animation::getParameter)
        .def("setParameter", +[](Animation * self, std::uint32_t param_id, int value){
            return self->setParamater(param_id, value); });

    py::class_<AnimationStorage>(m, "AnimationStorage")
        .def(py::init<>())
        .def("change", &AnimationStorage::change)
        .def("get", py::overload_cast<>(&AnimationStorage::get),
            py::return_value_policy::reference_internal);
}
