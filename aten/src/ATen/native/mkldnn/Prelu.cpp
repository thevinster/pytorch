#include <ATen/ATen.h>
#include <ATen/Config.h>
#include <ATen/NativeFunctions.h>

#if !AT_MKLDNN_ENABLED()

namespace at {
namespace native {

Tensor mkldnn_prelu(const Tensor& input, const Tensor& weight) {
  TORCH_CHECK(false, "mkldnn_prelu: ATen not compiled with MKLDNN support");
}

std::tuple<Tensor, Tensor> mkldnn_prelu_backward(
    const Tensor& grad_output,
    const Tensor& input,
    const Tensor& weight) {
  TORCH_CHECK(
      false, "mkldnn_prelu_backward: ATen not compiled with MKLDNN support");
}

} // namespace native
} // namespace at

#else // AT_MKLDNN_EBABLED

#include <ATen/native/mkldnn/MKLDNNCommon.h>
#include <ATen/native/mkldnn/Utils.h>

namespace at {
namespace native {

Tensor mkldnn_prelu(const Tensor& input, const Tensor& weight) {
  if (input.scalar_type() == ScalarType::BFloat16) {
    TORCH_CHECK(
        mkldnn_bf16_device_check(),
        "mkldnn_relu: bf16 path needs the cpu support avx512bw, avx512vl and avx512dq");
  }

  int64_t weight_num = weight.numel();
  if (weight_num != 1) {
    int64_t channel_size = input.dim() > 1 ? input.size(1) : 1;
    TORCH_CHECK(
        channel_size == weight_num,
        "Mismatch of parameter numbers and input channel size. Found parameter numbers = ",
        weight_num,
        " and channel size = ",
        channel_size,
        ".");
  }
  const ideep::tensor& x = itensor_from_mkldnn(input);
  const ideep::tensor& w = itensor_from_tensor(weight);

  ideep::tensor y;
  ideep::prelu_forward::compute(x, w, y, ideep::prop_kind::forward_training);
  return new_with_itensor_mkldnn(
      std::move(y),
      optTypeMetaToScalarType(input.options().dtype_opt()),
      input.options().device_opt());
}

std::tuple<Tensor, Tensor> mkldnn_prelu_backward(
    const Tensor& grad_output,
    const Tensor& input,
    const Tensor& weight) {
  const ideep::tensor& x = itensor_from_mkldnn(input);
  const ideep::tensor& w = itensor_from_tensor(weight);
  const ideep::tensor grady = itensor_from_mkldnn(grad_output);
  ideep::tensor gradx;
  ideep::tensor gradw;

  ideep::prelu_backward::compute(
      x, w, grady, gradx, gradw, ideep::prop_kind::backward);
  if (weight.is_mkldnn()) {
    return std::make_tuple(
        new_with_itensor_mkldnn(
            std::move(gradx),
            optTypeMetaToScalarType(grad_output.options().dtype_opt()),
            grad_output.options().device_opt()),
        new_with_itensor_mkldnn(
            std::move(gradw),
            optTypeMetaToScalarType(weight.options().dtype_opt()),
            weight.options().device_opt()));
  } else {
    return std::make_tuple(
        new_with_itensor_mkldnn(
            std::move(gradx),
            optTypeMetaToScalarType(grad_output.options().dtype_opt()),
            grad_output.options().device_opt()),
        mkldnn_to_dense(new_with_itensor_mkldnn(
            std::move(gradw),
            optTypeMetaToScalarType(weight.options().dtype_opt()),
            weight.options().device_opt())));
  }
}
} // namespace native
} // namespace at

#endif // AT_MKLDNN_EBABLED
