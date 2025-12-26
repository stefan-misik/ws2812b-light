#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/detail/common.h>
#include <pybind11/buffer_info.h>
#include <pybind11/stl.h>

#include "app/animation.hpp"
#include "app/animation_storage.hpp"
#include "led_strip.hpp"


namespace py = pybind11;


PYBIND11_MODULE(animations, m)
{
    py::enum_<Animation::DataType>(m, "DataType")
        .value("BOTH", Animation::DataType::BOTH)
        .value("ONLY_CONFIG", Animation::DataType::ONLY_CONFIG);


    py::class_<LedState>(m, "LedState")
        .def(py::init<>())

        .def(py::init<std::uint8_t, std::uint8_t, std::uint8_t>(),
            py::arg("red"), py::arg("green"), py::arg("blue"))

        .def(py::init<std::uint32_t>(), py::arg("hex_color"))

        .def_readwrite("red", &LedState::red)
        .def_readwrite("green", &LedState::green)
        .def_readwrite("blue", &LedState::blue);


    py::class_<AbstractLedStrip>(m, "LedStrip")
        .def(py::init<>([]() {
            return (new LedStrip<100>())->abstractPtr(); }),
            py::return_value_policy::take_ownership)

        .def_readonly("led_count", &AbstractLedStrip::led_count)

        .def("__len__", +[](AbstractLedStrip * ls) { return ls->led_count; })

        .def("__getitem__", +[](AbstractLedStrip * ls, std::size_t led_id) {
            if (led_id >= ls->led_count)
                throw py::index_error("index out of range");
            return std::ref(ls->leds[led_id]); },
            py::return_value_policy::reference_internal, py::arg("led_id"));


    py::class_<Animation>(m, "Animation")
        .def("get_parameter", &Animation::getParameter, py::arg("param_id"))

        .def("set_parameter", +[](Animation * self, std::uint32_t param_id, int value){
            return self->setParamater(param_id, value); }, py::arg("param_id"), py::arg("value"))

        .def("store", +[](Animation * self, py::buffer b, Animation::DataType t) {
            py::buffer_info info = b.request(true);
            if (info.ndim != 1 || info.itemsize != 1)
                throw std::runtime_error("Expected 1D buffer of bytes (itemsize=1)");
            return self->store(info.ptr, info.size, t); },
            py::arg("buffer"), py::arg("type"))

        .def("restore", +[](Animation * self, py::buffer b, Animation::DataType t) {
            py::buffer_info info = b.request(false);
            if (info.ndim != 1 || info.itemsize != 1)
                throw std::runtime_error("Expected 1D buffer of bytes (itemsize=1)");
            return self->restore(info.ptr, info.size, t); },
            py::arg("buffer"), py::arg("type"))

        .def("render", +[](Animation * self, AbstractLedStrip & strip) {
            self->render(&strip, {}); }, py::arg("led_strip"));


    py::class_<AnimationStorage>(m, "AnimationStorage")
        .def_readonly_static("ANIMATION_COUNT", &AnimationStorage::ANIMATION_COUNT)

        .def(py::init<>())

        .def("change", &AnimationStorage::change, py::arg("anim_id"))

        .def("get", py::overload_cast<>(&AnimationStorage::get),
            py::return_value_policy::reference_internal);
}
