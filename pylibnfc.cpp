
#include <Python.h>
#include <pybind11/pybind11.h>
#include <string_view>
#include <memory>
#include <array>
#include <iostream>
#include <thread>
#include <nfc/nfc.h>

namespace py = pybind11;

#define MODULE_NAME pylibnfc
#define MODULE_VERSION 0.1
constexpr std::string_view VERSION_STR = PYBIND11_TOSTRING(MODULE_VERSION);

template <auto delete_fn>
struct deleter_from_fn
{
  void operator()(auto *arg) const { delete_fn(arg); }
};

using NfcContextPtr = std::unique_ptr<nfc_context, deleter_from_fn<nfc_exit>>;
using NfcDevicePtr = std::unique_ptr<nfc_device, deleter_from_fn<nfc_close>>;

struct NfcDevice;
struct NfcContext;

struct NfcContext
{
  NfcContext() : ctx{make_context()} {}

  static NfcContextPtr make_context()
  {
    nfc_context *ctx = nullptr;
    nfc_init(&ctx);
    if (!ctx)
    {
      throw std::runtime_error("nfc_init failed");
    }
    return NfcContextPtr{ctx};
  }

  NfcDevicePtr open_device()
  {
    // Open device as reader
    nfc_device *device = nfc_open(ctx.get(), nullptr);
    if (!device)
    {
      throw std::runtime_error("nfc_open failed");
    }
    nfc_initiator_init(device);

    // Drop the field for a while
    nfc_device_set_property_bool(device, NP_ACTIVATE_FIELD, false);
    nfc_device_set_property_bool(device, NP_INFINITE_SELECT, false);

    // Configure the CRC and Parity settings
    nfc_device_set_property_bool(device, NP_HANDLE_CRC, true);
    nfc_device_set_property_bool(device, NP_HANDLE_PARITY, true);

    // Enable field so more power consuming cards can power themselves up
    nfc_device_set_property_bool(device, NP_ACTIVATE_FIELD, true);

    return NfcDevicePtr{device};
  }

  NfcContextPtr ctx;
};

struct NfcDevice
{
  uint64_t poll_for_tag(const uint8_t poll_number)
  {
    const uint8_t poll_period = 2; // 2 x 150 ms = 300 ms
    constexpr std::array<nfc_modulation, 1> modulations = {
        nfc_modulation{.nmt = NMT_ISO14443A, .nbr = NBR_106},
    };

    nfc_target tag;
    auto result = nfc_initiator_poll_target(
        device.get(), modulations.data(), modulations.size(), poll_number, poll_period, &tag);
    if (result < 0)
    {
      throw std::runtime_error("nfc_initiator_poll_target failed");
    }

    if (result >= 1)
    {
      uint64_t id = 0;
      for (size_t i{0}; i < tag.nti.nai.szUidLen; ++i)
      {
        id = (id << 8) | tag.nti.nai.abtUid[i];
      }
      return id;
    }

    return 0;
  }

  NfcDevicePtr device;
};

struct NfcMonitor
{
  NfcMonitor()
      : ctx(), device{ctx.open_device()}
  {
  }

  uint64_t poll_for_tag(const uint8_t poll_number) { return device.poll_for_tag(poll_number); }

  NfcContext ctx;
  NfcDevice device;
};

PYBIND11_MODULE(MODULE_NAME, m)
{
  m.attr("__version__") = py::str(VERSION_STR.data(), VERSION_STR.size());

  py::class_<NfcMonitor>(m, "NfcMonitor")
      .def(py::init<>())
      .def("poll_for_tag", &NfcMonitor::poll_for_tag, py::call_guard<py::gil_scoped_release>());
}
