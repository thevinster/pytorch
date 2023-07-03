import enum
from typing import Any, Callable, Dict, List, Optional, Set, Type

import torch
from torch.distributed.algorithms.join import Joinable, JoinHook
from torch.optim import Optimizer

class _ZeROJoinHook(JoinHook):
    zero: Any = ...
    def __init__(self, zero: Any) -> None: ...
    def main_hook(self) -> None: ...

class _DDPBucketAssignment:
    bucket_index: int
    parameters: List[torch.Tensor]
    offset: int
    device: torch.device
    tensor: Optional[torch.Tensor]

class _OverlapStatus(enum.IntEnum):
    UNINITIALIZED: int = ...
    DDP_HAS_REBUILT_BUCKETS: int = ...
    INITIALIZED: int = ...

class _OverlapInfo:
    status: Any = ...
    params_per_bucket: Any = ...
    params_per_rank: Any = ...
    offsets: Any = ...
    broadcast_handles: Any = ...
    bucket_index_to_future: Any = ...
    bucket_index_to_bucket: Any = ...
    bucket_indices_seen: Any = ...
    assigned_ranks_per_bucket: List[Set[int]] = ...
    total_size: int = ...
    shard_buckets: bool = ...
    def __init__(self) -> None: ...
    def wait_for_broadcasts(self) -> None: ...
    def clear_per_iter_info(self) -> None: ...

class ZeroRedundancyOptimizer(Optimizer, Joinable):
    functional_optim_map: Any = ...
    initialized: bool = ...
    process_group: Any = ...
    world_size: int = ...
    rank: int = ...
    global_rank: int = ...
    parameters_as_bucket_view: bool = ...
    optim: Any = ...
    _device_to_device_index: Dict[torch.device, int] = ...
    _overlap_with_ddp: bool = ...
    _overlap_info: _OverlapInfo = ...
    _buckets: List[List[torch.Tensor]] = ...
    _bucket_assignments_per_rank: List[Dict[int, _DDPBucketAssignment]] = ...
    def __init__(
        self,
        params: Any,
        optimizer_class: Type[Optimizer],
        process_group: Optional[Any] = ...,
        parameters_as_bucket_view: bool = ...,
        overlap_with_ddp: bool = ...,
        **defaults: Any,
    ) -> None: ...
    def add_param_group(self, param_group: dict) -> None: ...
    def consolidate_state_dict(self, to: int = ...) -> None: ...
    def step(
        self, closure: Optional[Callable[[], float]] = ..., **kwargs: Any
    ) -> Optional[float]: ...
    def load_state_dict(self, state_dict: Dict[str, Any]) -> None: ...
    def state_dict(self) -> Dict[str, Any]: ...
    def _local_step(
        self,
        gradients: Optional[List[Optional[torch.Tensor]]] = None,
        closure: Optional[Callable[[], float]] = None,
        **kwargs: Any,
    ) -> Optional[float]: ...
    def _get_assigned_rank(self, bucket_index: int) -> int: ...
    def _init_zero_for_overlap(self) -> None: ...
    def join_hook(self, **kwargs): ...
    @property
    def join_device(self) -> torch.device: ...
    def join_process_group(self) -> Any: ...
