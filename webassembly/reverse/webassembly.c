#include <math.h>
#include <string.h>

#include "/root/Downloads/CTF/qwb/webassembly0/webassembly.h"
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)

#define TRAP(x) (wasm_rt_trap(WASM_RT_TRAP_##x), 0)

#define FUNC_PROLOGUE                                            \
  if (++wasm_rt_call_stack_depth > WASM_RT_MAX_CALL_STACK_DEPTH) \
    TRAP(EXHAUSTION)

#define FUNC_EPILOGUE --wasm_rt_call_stack_depth

#define UNREACHABLE TRAP(UNREACHABLE)

#define CALL_INDIRECT(table, t, ft, x, ...)          \
  (LIKELY((x) < table.size && table.data[x].func &&  \
          table.data[x].func_type == func_types[ft]) \
       ? ((t)table.data[x].func)(__VA_ARGS__)        \
       : TRAP(CALL_INDIRECT))

#define MEMCHECK(mem, a, t)  \
  if (UNLIKELY((a) + sizeof(t) > mem->size)) TRAP(OOB)

#define DEFINE_LOAD(name, t1, t2, t3)              \
  static inline t3 name(wasm_rt_memory_t* mem, u64 addr) {   \
    MEMCHECK(mem, addr, t1);                       \
    t1 result;                                     \
    memcpy(&result, &mem->data[addr], sizeof(t1)); \
    return (t3)(t2)result;                         \
  }

#define DEFINE_STORE(name, t1, t2)                           \
  static inline void name(wasm_rt_memory_t* mem, u64 addr, t2 value) { \
    MEMCHECK(mem, addr, t1);                                 \
    t1 wrapped = (t1)value;                                  \
    memcpy(&mem->data[addr], &wrapped, sizeof(t1));          \
  }

DEFINE_LOAD(i32_load, u32, u32, u32);
DEFINE_LOAD(i64_load, u64, u64, u64);
DEFINE_LOAD(f32_load, f32, f32, f32);
DEFINE_LOAD(f64_load, f64, f64, f64);
DEFINE_LOAD(i32_load8_s, s8, s32, u32);
DEFINE_LOAD(i64_load8_s, s8, s64, u64);
DEFINE_LOAD(i32_load8_u, u8, u32, u32);
DEFINE_LOAD(i64_load8_u, u8, u64, u64);
DEFINE_LOAD(i32_load16_s, s16, s32, u32);
DEFINE_LOAD(i64_load16_s, s16, s64, u64);
DEFINE_LOAD(i32_load16_u, u16, u32, u32);
DEFINE_LOAD(i64_load16_u, u16, u64, u64);
DEFINE_LOAD(i64_load32_s, s32, s64, u64);
DEFINE_LOAD(i64_load32_u, u32, u64, u64);
DEFINE_STORE(i32_store, u32, u32);
DEFINE_STORE(i64_store, u64, u64);
DEFINE_STORE(f32_store, f32, f32);
DEFINE_STORE(f64_store, f64, f64);
DEFINE_STORE(i32_store8, u8, u32);
DEFINE_STORE(i32_store16, u16, u32);
DEFINE_STORE(i64_store8, u8, u64);
DEFINE_STORE(i64_store16, u16, u64);
DEFINE_STORE(i64_store32, u32, u64);

#define I32_CLZ(x) ((x) ? __builtin_clz(x) : 32)
#define I64_CLZ(x) ((x) ? __builtin_clzll(x) : 64)
#define I32_CTZ(x) ((x) ? __builtin_ctz(x) : 32)
#define I64_CTZ(x) ((x) ? __builtin_ctzll(x) : 64)
#define I32_POPCNT(x) (__builtin_popcount(x))
#define I64_POPCNT(x) (__builtin_popcountll(x))

#define DIV_S(ut, min, x, y)                                 \
   ((UNLIKELY((y) == 0)) ?                TRAP(DIV_BY_ZERO)  \
  : (UNLIKELY((x) == min && (y) == -1)) ? TRAP(INT_OVERFLOW) \
  : (ut)((x) / (y)))

#define REM_S(ut, min, x, y)                                \
   ((UNLIKELY((y) == 0)) ?                TRAP(DIV_BY_ZERO) \
  : (UNLIKELY((x) == min && (y) == -1)) ? 0                 \
  : (ut)((x) % (y)))

#define I32_DIV_S(x, y) DIV_S(u32, INT32_MIN, (s32)x, (s32)y)
#define I64_DIV_S(x, y) DIV_S(u64, INT64_MIN, (s64)x, (s64)y)
#define I32_REM_S(x, y) REM_S(u32, INT32_MIN, (s32)x, (s32)y)
#define I64_REM_S(x, y) REM_S(u64, INT64_MIN, (s64)x, (s64)y)

#define DIVREM_U(op, x, y) \
  ((UNLIKELY((y) == 0)) ? TRAP(DIV_BY_ZERO) : ((x) op (y)))

#define DIV_U(x, y) DIVREM_U(/, x, y)
#define REM_U(x, y) DIVREM_U(%, x, y)

#define ROTL(x, y, mask) \
  (((x) << ((y) & (mask))) | ((x) >> (((mask) - (y) + 1) & (mask))))
#define ROTR(x, y, mask) \
  (((x) >> ((y) & (mask))) | ((x) << (((mask) - (y) + 1) & (mask))))

#define I32_ROTL(x, y) ROTL(x, y, 31)
#define I64_ROTL(x, y) ROTL(x, y, 63)
#define I32_ROTR(x, y) ROTR(x, y, 31)
#define I64_ROTR(x, y) ROTR(x, y, 63)

#define FMIN(x, y)                                          \
   ((UNLIKELY((x) != (x))) ? NAN                            \
  : (UNLIKELY((y) != (y))) ? NAN                            \
  : (UNLIKELY((x) == 0 && (y) == 0)) ? (signbit(x) ? x : y) \
  : (x < y) ? x : y)

#define FMAX(x, y)                                          \
   ((UNLIKELY((x) != (x))) ? NAN                            \
  : (UNLIKELY((y) != (y))) ? NAN                            \
  : (UNLIKELY((x) == 0 && (y) == 0)) ? (signbit(x) ? y : x) \
  : (x > y) ? x : y)

#define TRUNC_S(ut, st, ft, min, max, maxop, x)                             \
   ((UNLIKELY((x) != (x))) ? TRAP(INVALID_CONVERSION)                       \
  : (UNLIKELY((x) < (ft)(min) || (x) maxop (ft)(max))) ? TRAP(INT_OVERFLOW) \
  : (ut)(st)(x))

#define I32_TRUNC_S_F32(x) TRUNC_S(u32, s32, f32, INT32_MIN, INT32_MAX, >=, x)
#define I64_TRUNC_S_F32(x) TRUNC_S(u64, s64, f32, INT64_MIN, INT64_MAX, >=, x)
#define I32_TRUNC_S_F64(x) TRUNC_S(u32, s32, f64, INT32_MIN, INT32_MAX, >,  x)
#define I64_TRUNC_S_F64(x) TRUNC_S(u64, s64, f64, INT64_MIN, INT64_MAX, >=, x)

#define TRUNC_U(ut, ft, max, maxop, x)                                    \
   ((UNLIKELY((x) != (x))) ? TRAP(INVALID_CONVERSION)                     \
  : (UNLIKELY((x) <= (ft)-1 || (x) maxop (ft)(max))) ? TRAP(INT_OVERFLOW) \
  : (ut)(x))

#define I32_TRUNC_U_F32(x) TRUNC_U(u32, f32, UINT32_MAX, >=, x)
#define I64_TRUNC_U_F32(x) TRUNC_U(u64, f32, UINT64_MAX, >=, x)
#define I32_TRUNC_U_F64(x) TRUNC_U(u32, f64, UINT32_MAX, >,  x)
#define I64_TRUNC_U_F64(x) TRUNC_U(u64, f64, UINT64_MAX, >=, x)

#define DEFINE_REINTERPRET(name, t1, t2)  \
  static inline t2 name(t1 x) {           \
    t2 result;                            \
    memcpy(&result, &x, sizeof(result));  \
    return result;                        \
  }

DEFINE_REINTERPRET(f32_reinterpret_i32, u32, f32)
DEFINE_REINTERPRET(i32_reinterpret_f32, f32, u32)
DEFINE_REINTERPRET(f64_reinterpret_i64, u64, f64)
DEFINE_REINTERPRET(i64_reinterpret_f64, f64, u64)


static u32 func_types[22];

static void init_func_types(void) {
  func_types[0] = wasm_rt_register_func_type(3, 1, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[1] = wasm_rt_register_func_type(1, 1, WASM_RT_I32, WASM_RT_I32);
  func_types[2] = wasm_rt_register_func_type(1, 0, WASM_RT_I32);
  func_types[3] = wasm_rt_register_func_type(2, 1, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[4] = wasm_rt_register_func_type(0, 1, WASM_RT_I32);
  func_types[5] = wasm_rt_register_func_type(2, 0, WASM_RT_I32, WASM_RT_I32);
  func_types[6] = wasm_rt_register_func_type(2, 1, WASM_RT_I32, WASM_RT_I32, WASM_RT_I64);
  func_types[7] = wasm_rt_register_func_type(3, 0, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[8] = wasm_rt_register_func_type(5, 1, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[9] = wasm_rt_register_func_type(3, 1, WASM_RT_I64, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[10] = wasm_rt_register_func_type(2, 1, WASM_RT_I64, WASM_RT_I32, WASM_RT_I32);
  func_types[11] = wasm_rt_register_func_type(5, 0, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[12] = wasm_rt_register_func_type(6, 1, WASM_RT_I32, WASM_RT_F64, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[13] = wasm_rt_register_func_type(2, 1, WASM_RT_F64, WASM_RT_I32, WASM_RT_F64);
  func_types[14] = wasm_rt_register_func_type(3, 0, WASM_RT_I32, WASM_RT_I32, WASM_RT_I64);
  func_types[15] = wasm_rt_register_func_type(2, 1, WASM_RT_I32, WASM_RT_I32, WASM_RT_F64);
  func_types[16] = wasm_rt_register_func_type(4, 1, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_F64);
  func_types[17] = wasm_rt_register_func_type(5, 1, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_F64);
  func_types[18] = wasm_rt_register_func_type(1, 1, WASM_RT_I32, WASM_RT_I64);
  func_types[19] = wasm_rt_register_func_type(2, 1, WASM_RT_F64, WASM_RT_F64, WASM_RT_F64);
  func_types[20] = wasm_rt_register_func_type(0, 0);
  func_types[21] = wasm_rt_register_func_type(4, 1, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
}

static u32 stackAlloc(u32);
static u32 stackSave(void);
static void stackRestore(u32);
static void establishStackSpace(u32, u32);
static void f15(u32);
static u32 _main(void);
static u32 _malloc(u32);
static void _free(u32);
static u32 f19(u32, u32);
static u32 f20(u32, u32);
static void f21(u32, u32);
static u32 f22(u32);
static u32 f23(u32, u32, u32);
static u32 f24(u32, u32, u32);
static u32 f25(u32);
static u32 ___errno_location(void);
static u32 f27(u32, u32, u32);
static u32 f28(u32, u32, u32);
static u32 f29(u32);
static void f30(u32, u32);
static u64 f31(u32, u32);
static u32 f32_0(u32);
static u32 f33(u32);
static u32 f34(u32);
static u32 f35(u32);
static u32 f36(u32, u32);
static u32 f37(u32, u32, u32);
static u32 f38(u32, u32);
static u32 f39(u32, u32);
static void f40(u32, u32, u32);
static u32 f41(u32, u32, u32, u32, u32);
static void f42(u32, u32, u32);
static u32 f43(u32);
static void f44(u32, u32, u32);
static u32 f45(u64, u32, u32);
static u32 f46(u64, u32);
static u32 f47(u64, u32);
static void f48(u32, u32, u32, u32, u32);
static u32 f49(u32, f64, u32, u32, u32, u32);
static f64 f50(f64, u32);
static void f51(u32, u32, u32);
static u32 f52(u32);
static void f53(u32);
static void f54(u32, u32);
static u32 f55(u32, u32);
static void f56(u32, u32, u64);
static f64 f57(u32, u32);
static f64 f58(u32, u32, u32, u32);
static f64 f59(u32, u32, u32, u32, u32);
static u64 f60(u32);
static f64 f61(f64, u32);
static f64 f62(f64, f64);
static f64 f63(f64, f64);
static f64 f64_0(f64, u32);
static f64 f65(f64, f64);
static void f66(u32, u32);
static void f67(void);
static void f68(u32);
static u32 _memcpy(u32, u32, u32);
static u32 _memset(u32, u32, u32);
static u32 _sbrk(u32);
static u32 dynCall_ii(u32, u32);
static u32 dynCall_iiii(u32, u32, u32, u32);
static u32 f74(u32);
static u32 f75(u32, u32, u32);

static u32 g4;
static f64 g5;
static f64 g6;
static u32 g7;
static u32 g8;

static void init_globals(void) {
  g4 = (*Z_envZ_DYNAMICTOP_PTRZ_i);
  g5 = (*Z_globalZ_NaNZ_d);
  g6 = (*Z_globalZ_InfinityZ_d);
  g7 = 6560u;
  g8 = 5249440u;
}

static u32 stackAlloc(u32 p0) {
  u32 l1 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1;
  i0 = g7;
  l1 = i0;
  i0 = p0;
  i1 = g7;
  i0 += i1;
  g7 = i0;
  i0 = g7;
  i1 = 15u;
  i0 += i1;
  i1 = 4294967280u;
  i0 &= i1;
  g7 = i0;
  i0 = l1;
  FUNC_EPILOGUE;
  return i0;
}

static u32 stackSave(void) {
  FUNC_PROLOGUE;
  u32 i0;
  i0 = g7;
  FUNC_EPILOGUE;
  return i0;
}

static void stackRestore(u32 p0) {
  FUNC_PROLOGUE;
  u32 i0;
  i0 = p0;
  g7 = i0;
  FUNC_EPILOGUE;
}

static void establishStackSpace(u32 p0, u32 p1) {
  FUNC_PROLOGUE;
  u32 i0;
  i0 = p0;
  g7 = i0;
  i0 = p1;
  g8 = i0;
  FUNC_EPILOGUE;
}

static void f15(u32 p0) {
  u32 l1 = 0, l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0, 
      l9 = 0, l10 = 0, l11 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5, i6;
  u64 j1;
  i0 = g7;
  l7 = i0;
  i0 = g7;
  i1 = 32u;
  i0 += i1;
  g7 = i0;
  i0 = l7;
  i1 = 24u;
  i0 += i1;
  l10 = i0;
  i0 = l7;
  i1 = 16u;
  i0 += i1;
  l11 = i0;
  i0 = l7;
  l5 = i0;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0), j1);
  i0 = l5;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0 + 8), j1);
  i0 = p0;
  i1 = 4u;
  i0 += i1;
  l4 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l6 = i0;
  L0: 
    i0 = l1;
    i1 = l6;
    i2 = l2;
    i3 = l2;
    i4 = 3u;
    i3 &= i4;
    i4 = 2u;
    i3 <<= (i4 & 31);
    i4 = l5;
    i3 += i4;
    i3 = i32_load(Z_envZ_memory, (u64)(i3));
    i2 += i3;
    i3 = l1;
    i4 = l1;
    i5 = 4u;
    i4 <<= (i5 & 31);
    i5 = l1;
    i6 = 5u;
    i5 >>= (i6 & 31);
    i4 ^= i5;
    i3 += i4;
    i2 ^= i3;
    i1 += i2;
    l6 = i1;
    i2 = l6;
    i3 = 4u;
    i2 <<= (i3 & 31);
    i3 = l6;
    i4 = 5u;
    i3 >>= (i4 & 31);
    i2 ^= i3;
    i1 += i2;
    i2 = l2;
    i3 = 2654435769u;
    i2 += i3;
    l2 = i2;
    i3 = l2;
    i4 = 11u;
    i3 >>= (i4 & 31);
    i4 = 3u;
    i3 &= i4;
    i4 = 2u;
    i3 <<= (i4 & 31);
    i4 = l5;
    i3 += i4;
    i3 = i32_load(Z_envZ_memory, (u64)(i3));
    i2 += i3;
    i1 ^= i2;
    i0 += i1;
    l1 = i0;
    i0 = l3;
    i1 = 1u;
    i0 += i1;
    l3 = i0;
    i1 = 32u;
    i0 = i0 != i1;
    if (i0) {goto L0;}
  i0 = p0;
  i1 = l6;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l4;
  i1 = l1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 0u;
  l2 = i0;
  i0 = p0;
  i1 = 12u;
  i0 += i1;
  l8 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i0 = p0;
  i1 = 8u;
  i0 += i1;
  l9 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l3 = i0;
  i0 = 0u;
  l4 = i0;
  L1: 
    i0 = l1;
    i1 = l3;
    i2 = l1;
    i3 = l1;
    i4 = 4u;
    i3 <<= (i4 & 31);
    i4 = l1;
    i5 = 5u;
    i4 >>= (i5 & 31);
    i3 ^= i4;
    i2 += i3;
    i3 = l2;
    i4 = l2;
    i5 = 3u;
    i4 &= i5;
    i5 = 2u;
    i4 <<= (i5 & 31);
    i5 = l5;
    i4 += i5;
    i4 = i32_load(Z_envZ_memory, (u64)(i4));
    i3 += i4;
    i2 ^= i3;
    i1 += i2;
    l3 = i1;
    i2 = l3;
    i3 = 4u;
    i2 <<= (i3 & 31);
    i3 = l3;
    i4 = 5u;
    i3 >>= (i4 & 31);
    i2 ^= i3;
    i1 += i2;
    i2 = l2;
    i3 = 2654435769u;
    i2 += i3;
    l2 = i2;
    i3 = l2;
    i4 = 11u;
    i3 >>= (i4 & 31);
    i4 = 3u;
    i3 &= i4;
    i4 = 2u;
    i3 <<= (i4 & 31);
    i4 = l5;
    i3 += i4;
    i3 = i32_load(Z_envZ_memory, (u64)(i3));
    i2 += i3;
    i1 ^= i2;
    i0 += i1;
    l1 = i0;
    i0 = l4;
    i1 = 1u;
    i0 += i1;
    l4 = i0;
    i1 = 32u;
    i0 = i0 != i1;
    if (i0) {goto L1;}
  i0 = l9;
  i1 = l3;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l8;
  i1 = l1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 0u;
  l2 = i0;
  i0 = p0;
  i1 = 20u;
  i0 += i1;
  l8 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i0 = p0;
  i1 = 16u;
  i0 += i1;
  l9 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l3 = i0;
  i0 = 0u;
  l4 = i0;
  L2: 
    i0 = l1;
    i1 = l3;
    i2 = l1;
    i3 = l1;
    i4 = 4u;
    i3 <<= (i4 & 31);
    i4 = l1;
    i5 = 5u;
    i4 >>= (i5 & 31);
    i3 ^= i4;
    i2 += i3;
    i3 = l2;
    i4 = l2;
    i5 = 3u;
    i4 &= i5;
    i5 = 2u;
    i4 <<= (i5 & 31);
    i5 = l5;
    i4 += i5;
    i4 = i32_load(Z_envZ_memory, (u64)(i4));
    i3 += i4;
    i2 ^= i3;
    i1 += i2;
    l3 = i1;
    i2 = l3;
    i3 = 4u;
    i2 <<= (i3 & 31);
    i3 = l3;
    i4 = 5u;
    i3 >>= (i4 & 31);
    i2 ^= i3;
    i1 += i2;
    i2 = l2;
    i3 = 2654435769u;
    i2 += i3;
    l2 = i2;
    i3 = l2;
    i4 = 11u;
    i3 >>= (i4 & 31);
    i4 = 3u;
    i3 &= i4;
    i4 = 2u;
    i3 <<= (i4 & 31);
    i4 = l5;
    i3 += i4;
    i3 = i32_load(Z_envZ_memory, (u64)(i3));
    i2 += i3;
    i1 ^= i2;
    i0 += i1;
    l1 = i0;
    i0 = l4;
    i1 = 1u;
    i0 += i1;
    l4 = i0;
    i1 = 32u;
    i0 = i0 != i1;
    if (i0) {goto L2;}
  i0 = l9;
  i1 = l3;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l8;
  i1 = l1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 0u;
  l2 = i0;
  i0 = p0;
  i1 = 28u;
  i0 += i1;
  l8 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i0 = p0;
  i1 = 24u;
  i0 += i1;
  l9 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l3 = i0;
  i0 = 0u;
  l4 = i0;
  L3: 
    i0 = l1;
    i1 = l3;
    i2 = l1;
    i3 = l1;
    i4 = 4u;
    i3 <<= (i4 & 31);
    i4 = l1;
    i5 = 5u;
    i4 >>= (i5 & 31);
    i3 ^= i4;
    i2 += i3;
    i3 = l2;
    i4 = l2;
    i5 = 3u;
    i4 &= i5;
    i5 = 2u;
    i4 <<= (i5 & 31);
    i5 = l5;
    i4 += i5;
    i4 = i32_load(Z_envZ_memory, (u64)(i4));
    i3 += i4;
    i2 ^= i3;
    i1 += i2;
    l3 = i1;
    i2 = l3;
    i3 = 4u;
    i2 <<= (i3 & 31);
    i3 = l3;
    i4 = 5u;
    i3 >>= (i4 & 31);
    i2 ^= i3;
    i1 += i2;
    i2 = l2;
    i3 = 2654435769u;
    i2 += i3;
    l2 = i2;
    i3 = 11u;
    i2 >>= (i3 & 31);
    i3 = 3u;
    i2 &= i3;
    i3 = 2u;
    i2 <<= (i3 & 31);
    i3 = l5;
    i2 += i3;
    i2 = i32_load(Z_envZ_memory, (u64)(i2));
    i3 = l2;
    i2 += i3;
    i1 ^= i2;
    i0 += i1;
    l1 = i0;
    i0 = l4;
    i1 = 1u;
    i0 += i1;
    l4 = i0;
    i1 = 32u;
    i0 = i0 != i1;
    if (i0) {goto L3;}
  i0 = l9;
  i1 = l3;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l8;
  i1 = l1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 1));
  i1 = 4294967190u;
  i0 ^= i1;
  i1 = 255u;
  i0 &= i1;
  i1 = l6;
  i2 = 255u;
  i1 &= i2;
  i2 = 149u;
  i1 ^= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 2));
  i2 = 104u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 3));
  i2 = 4294967271u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 4));
  i2 = 4294967223u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 5));
  i2 = 85u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 6));
  i2 = 23u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 7));
  i2 = 4294967241u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 8));
  i2 = 4294967213u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 9));
  i2 = 3u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 10));
  i2 = 30u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 11));
  i2 = 4294967247u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 12));
  i2 = 111u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 13));
  i2 = 4294967237u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 14));
  i2 = 97u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 15));
  i2 = 75u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 16));
  i2 = 2u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 17));
  i2 = 4294967184u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 18));
  i2 = 4294967293u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 19));
  i2 = 45u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 20));
  i2 = 34u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 21));
  i2 = 4294967277u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 22));
  i2 = 10u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 23));
  i2 = 4294967187u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 24));
  i2 = 48u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 25));
  i2 = 126u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 26));
  i2 = 4294967241u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 27));
  i2 = 4294967276u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 28));
  i2 = 4294967180u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 29));
  i2 = 4294967190u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 30));
  i2 = 4294967217u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 31));
  i2 = 4294967264u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 32));
  i2 = 101u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 33));
  i2 = 54u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 34));
  i2 = 56u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 35));
  i2 = 98u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = p0;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 36));
  i2 = 98u;
  i1 ^= i2;
  i2 = 255u;
  i1 &= i2;
  i0 += i1;
  i1 = 0u;
  i2 = p0;
  i2 = i32_load8_s(Z_envZ_memory, (u64)(i2 + 37));
  i3 = 125u;
  i2 ^= i3;
  i3 = 255u;
  i2 &= i3;
  i1 -= i2;
  i0 = i0 == i1;
  if (i0) {
    i0 = 2524u;
    i1 = l11;
    f66(i0, i1);
    f67();
    i0 = l7;
    g7 = i0;
    i0 = 0u;
  } else {
    i0 = 2531u;
    i1 = l10;
    f66(i0, i1);
    f67();
    i0 = l7;
    g7 = i0;
    i0 = 0u;
  }
  FUNC_EPILOGUE;
}

static u32 _main(void) {
  u32 l0 = 0, l1 = 0, l2 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1;
  i0 = g7;
  l0 = i0;
  i0 = g7;
  i1 = 48u;
  i0 += i1;
  g7 = i0;
  i0 = l0;
  i1 = 40u;
  i0 += i1;
  l2 = i0;
  i1 = l0;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = g7;
  l1 = i0;
  i0 = g7;
  i1 = 16u;
  i0 += i1;
  g7 = i0;
  i0 = l1;
  i1 = l2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 2144u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  i1 = l1;
  f54(i0, i1);
  i0 = l1;
  g7 = i0;
  i0 = l0;
  f15(i0);
  i0 = l0;
  g7 = i0;
  i0 = 0u;
  FUNC_EPILOGUE;
  return i0;
}

static u32 _malloc(u32 p0) {
  u32 l1 = 0, l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0, 
      l9 = 0, l10 = 0, l11 = 0, l12 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5, i6;
  u64 j1;
  i0 = g7;
  l10 = i0;
  i0 = g7;
  i1 = 16u;
  i0 += i1;
  g7 = i0;
  i0 = p0;
  i1 = 245u;
  i0 = i0 < i1;
  if (i0) {
    i0 = 4704u;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l5 = i0;
    i1 = 16u;
    i2 = p0;
    i3 = 11u;
    i2 += i3;
    i3 = 4294967288u;
    i2 &= i3;
    i3 = p0;
    i4 = 11u;
    i3 = i3 < i4;
    i1 = i3 ? i1 : i2;
    l2 = i1;
    i2 = 3u;
    i1 >>= (i2 & 31);
    p0 = i1;
    i0 >>= (i1 & 31);
    l1 = i0;
    i1 = 3u;
    i0 &= i1;
    if (i0) {
      i0 = l1;
      i1 = 1u;
      i0 &= i1;
      i1 = 1u;
      i0 ^= i1;
      i1 = p0;
      i0 += i1;
      l1 = i0;
      i1 = 3u;
      i0 <<= (i1 & 31);
      i1 = 4744u;
      i0 += i1;
      l2 = i0;
      i1 = 8u;
      i0 += i1;
      l4 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      i1 = 8u;
      i0 += i1;
      l6 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      p0 = i0;
      i1 = l2;
      i0 = i0 == i1;
      if (i0) {
        i0 = 4704u;
        i1 = 1u;
        i2 = l1;
        i1 <<= (i2 & 31);
        i2 = 4294967295u;
        i1 ^= i2;
        i2 = l5;
        i1 &= i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      } else {
        i0 = p0;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
        i0 = l4;
        i1 = p0;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
      i0 = l3;
      i1 = l1;
      i2 = 3u;
      i1 <<= (i2 & 31);
      p0 = i1;
      i2 = 3u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
      i0 = p0;
      i1 = l3;
      i0 += i1;
      i1 = 4u;
      i0 += i1;
      p0 = i0;
      i1 = p0;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l10;
      g7 = i0;
      i0 = l6;
      goto Bfunc;
    }
    i0 = l2;
    i1 = 4712u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    l7 = i1;
    i0 = i0 > i1;
    if (i0) {
      i0 = l1;
      if (i0) {
        i0 = l1;
        i1 = p0;
        i0 <<= (i1 & 31);
        i1 = 2u;
        i2 = p0;
        i1 <<= (i2 & 31);
        p0 = i1;
        i2 = 0u;
        i3 = p0;
        i2 -= i3;
        i1 |= i2;
        i0 &= i1;
        p0 = i0;
        i1 = 0u;
        i2 = p0;
        i1 -= i2;
        i0 &= i1;
        i1 = 4294967295u;
        i0 += i1;
        p0 = i0;
        i1 = 12u;
        i0 >>= (i1 & 31);
        i1 = 16u;
        i0 &= i1;
        l1 = i0;
        i1 = p0;
        i2 = l1;
        i1 >>= (i2 & 31);
        p0 = i1;
        i2 = 5u;
        i1 >>= (i2 & 31);
        i2 = 8u;
        i1 &= i2;
        l1 = i1;
        i0 |= i1;
        i1 = p0;
        i2 = l1;
        i1 >>= (i2 & 31);
        p0 = i1;
        i2 = 2u;
        i1 >>= (i2 & 31);
        i2 = 4u;
        i1 &= i2;
        l1 = i1;
        i0 |= i1;
        i1 = p0;
        i2 = l1;
        i1 >>= (i2 & 31);
        p0 = i1;
        i2 = 1u;
        i1 >>= (i2 & 31);
        i2 = 2u;
        i1 &= i2;
        l1 = i1;
        i0 |= i1;
        i1 = p0;
        i2 = l1;
        i1 >>= (i2 & 31);
        p0 = i1;
        i2 = 1u;
        i1 >>= (i2 & 31);
        i2 = 1u;
        i1 &= i2;
        l1 = i1;
        i0 |= i1;
        i1 = p0;
        i2 = l1;
        i1 >>= (i2 & 31);
        i0 += i1;
        l3 = i0;
        i1 = 3u;
        i0 <<= (i1 & 31);
        i1 = 4744u;
        i0 += i1;
        l4 = i0;
        i1 = 8u;
        i0 += i1;
        l6 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l1 = i0;
        i1 = 8u;
        i0 += i1;
        l8 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        p0 = i0;
        i1 = l4;
        i0 = i0 == i1;
        if (i0) {
          i0 = 4704u;
          i1 = 1u;
          i2 = l3;
          i1 <<= (i2 & 31);
          i2 = 4294967295u;
          i1 ^= i2;
          i2 = l5;
          i1 &= i2;
          p0 = i1;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
        } else {
          i0 = p0;
          i1 = l4;
          i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
          i0 = l6;
          i1 = p0;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l5;
          p0 = i0;
        }
        i0 = l1;
        i1 = l2;
        i2 = 3u;
        i1 |= i2;
        i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
        i0 = l1;
        i1 = l2;
        i0 += i1;
        l4 = i0;
        i1 = l3;
        i2 = 3u;
        i1 <<= (i2 & 31);
        l3 = i1;
        i2 = l2;
        i1 -= i2;
        l5 = i1;
        i2 = 1u;
        i1 |= i2;
        i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
        i0 = l1;
        i1 = l3;
        i0 += i1;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l7;
        if (i0) {
          i0 = 4724u;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l3 = i0;
          i0 = l7;
          i1 = 3u;
          i0 >>= (i1 & 31);
          l2 = i0;
          i1 = 3u;
          i0 <<= (i1 & 31);
          i1 = 4744u;
          i0 += i1;
          l1 = i0;
          i0 = 1u;
          i1 = l2;
          i0 <<= (i1 & 31);
          l2 = i0;
          i1 = p0;
          i0 &= i1;
          if (i0) {
            i0 = l1;
            i1 = 8u;
            i0 += i1;
            l2 = i0;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
          } else {
            i0 = 4704u;
            i1 = p0;
            i2 = l2;
            i1 |= i2;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            i0 = l1;
            i1 = 8u;
            i0 += i1;
            l2 = i0;
            i0 = l1;
          }
          p0 = i0;
          i0 = l2;
          i1 = l3;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = p0;
          i1 = l3;
          i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
          i0 = l3;
          i1 = p0;
          i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
          i0 = l3;
          i1 = l1;
          i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
        }
        i0 = 4712u;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = 4724u;
        i1 = l4;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l10;
        g7 = i0;
        i0 = l8;
        goto Bfunc;
      }
      i0 = 4708u;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l11 = i0;
      if (i0) {
        i0 = 0u;
        i1 = l11;
        i0 -= i1;
        i1 = l11;
        i0 &= i1;
        i1 = 4294967295u;
        i0 += i1;
        p0 = i0;
        i1 = 12u;
        i0 >>= (i1 & 31);
        i1 = 16u;
        i0 &= i1;
        l1 = i0;
        i1 = p0;
        i2 = l1;
        i1 >>= (i2 & 31);
        p0 = i1;
        i2 = 5u;
        i1 >>= (i2 & 31);
        i2 = 8u;
        i1 &= i2;
        l1 = i1;
        i0 |= i1;
        i1 = p0;
        i2 = l1;
        i1 >>= (i2 & 31);
        p0 = i1;
        i2 = 2u;
        i1 >>= (i2 & 31);
        i2 = 4u;
        i1 &= i2;
        l1 = i1;
        i0 |= i1;
        i1 = p0;
        i2 = l1;
        i1 >>= (i2 & 31);
        p0 = i1;
        i2 = 1u;
        i1 >>= (i2 & 31);
        i2 = 2u;
        i1 &= i2;
        l1 = i1;
        i0 |= i1;
        i1 = p0;
        i2 = l1;
        i1 >>= (i2 & 31);
        p0 = i1;
        i2 = 1u;
        i1 >>= (i2 & 31);
        i2 = 1u;
        i1 &= i2;
        l1 = i1;
        i0 |= i1;
        i1 = p0;
        i2 = l1;
        i1 >>= (i2 & 31);
        i0 += i1;
        i1 = 2u;
        i0 <<= (i1 & 31);
        i1 = 5008u;
        i0 += i1;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l3 = i0;
        l1 = i0;
        i0 = l3;
        i0 = i32_load(Z_envZ_memory, (u64)(i0 + 4));
        i1 = 4294967288u;
        i0 &= i1;
        i1 = l2;
        i0 -= i1;
        l8 = i0;
        L9: 
          i0 = l1;
          i0 = i32_load(Z_envZ_memory, (u64)(i0 + 16));
          p0 = i0;
          i0 = !(i0);
          if (i0) {
            i0 = l1;
            i0 = i32_load(Z_envZ_memory, (u64)(i0 + 20));
            p0 = i0;
            i0 = !(i0);
            if (i0) {goto B10;}
          }
          i0 = p0;
          l1 = i0;
          i1 = l3;
          i2 = l1;
          i2 = i32_load(Z_envZ_memory, (u64)(i2 + 4));
          i3 = 4294967288u;
          i2 &= i3;
          i3 = l2;
          i2 -= i3;
          p0 = i2;
          i3 = l8;
          i2 = i2 < i3;
          l4 = i2;
          i0 = i2 ? i0 : i1;
          l3 = i0;
          i0 = p0;
          i1 = l8;
          i2 = l4;
          i0 = i2 ? i0 : i1;
          l8 = i0;
          goto L9;
          B10:;
        i0 = l2;
        i1 = l3;
        i0 += i1;
        l12 = i0;
        i1 = l3;
        i0 = i0 > i1;
        if (i0) {
          i0 = l3;
          i0 = i32_load(Z_envZ_memory, (u64)(i0 + 24));
          l9 = i0;
          i0 = l3;
          i1 = l3;
          i1 = i32_load(Z_envZ_memory, (u64)(i1 + 12));
          p0 = i1;
          i0 = i0 == i1;
          if (i0) {
            i0 = l3;
            i1 = 20u;
            i0 += i1;
            l1 = i0;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
            p0 = i0;
            i0 = !(i0);
            if (i0) {
              i0 = l3;
              i1 = 16u;
              i0 += i1;
              l1 = i0;
              i0 = i32_load(Z_envZ_memory, (u64)(i0));
              p0 = i0;
              i0 = !(i0);
              if (i0) {
                i0 = 0u;
                p0 = i0;
                goto B14;
              }
            }
            L17: 
              i0 = p0;
              i1 = 20u;
              i0 += i1;
              l4 = i0;
              i0 = i32_load(Z_envZ_memory, (u64)(i0));
              l6 = i0;
              if (i0) {
                i0 = l4;
                l1 = i0;
                i0 = l6;
              } else {
                i0 = p0;
                i1 = 16u;
                i0 += i1;
                l4 = i0;
                i0 = i32_load(Z_envZ_memory, (u64)(i0));
                l6 = i0;
                i0 = !(i0);
                if (i0) {goto B18;}
                i0 = l4;
                l1 = i0;
                i0 = l6;
              }
              p0 = i0;
              goto L17;
              B18:;
            i0 = l1;
            i1 = 0u;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            B14:;
          } else {
            i0 = l3;
            i0 = i32_load(Z_envZ_memory, (u64)(i0 + 8));
            l1 = i0;
            i1 = p0;
            i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
            i0 = p0;
            i1 = l1;
            i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
          }
          i0 = l9;
          if (i0) {
            i0 = l3;
            i1 = l3;
            i1 = i32_load(Z_envZ_memory, (u64)(i1 + 28));
            l1 = i1;
            i2 = 2u;
            i1 <<= (i2 & 31);
            i2 = 5008u;
            i1 += i2;
            l4 = i1;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            i0 = i0 == i1;
            if (i0) {
              i0 = l4;
              i1 = p0;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
              i0 = p0;
              i0 = !(i0);
              if (i0) {
                i0 = 4708u;
                i1 = 1u;
                i2 = l1;
                i1 <<= (i2 & 31);
                i2 = 4294967295u;
                i1 ^= i2;
                i2 = l11;
                i1 &= i2;
                i32_store(Z_envZ_memory, (u64)(i0), i1);
                goto B21;
              }
            } else {
              i0 = l9;
              i1 = 16u;
              i0 += i1;
              l1 = i0;
              i1 = l9;
              i2 = 20u;
              i1 += i2;
              i2 = l3;
              i3 = l1;
              i3 = i32_load(Z_envZ_memory, (u64)(i3));
              i2 = i2 == i3;
              i0 = i2 ? i0 : i1;
              i1 = p0;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
              i0 = p0;
              i0 = !(i0);
              if (i0) {goto B21;}
            }
            i0 = p0;
            i1 = l9;
            i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
            i0 = l3;
            i0 = i32_load(Z_envZ_memory, (u64)(i0 + 16));
            l1 = i0;
            if (i0) {
              i0 = p0;
              i1 = l1;
              i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
              i0 = l1;
              i1 = p0;
              i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
            }
            i0 = l3;
            i0 = i32_load(Z_envZ_memory, (u64)(i0 + 20));
            l1 = i0;
            if (i0) {
              i0 = p0;
              i1 = l1;
              i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
              i0 = l1;
              i1 = p0;
              i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
            }
            B21:;
          }
          i0 = l8;
          i1 = 16u;
          i0 = i0 < i1;
          if (i0) {
            i0 = l3;
            i1 = l2;
            i2 = l8;
            i1 += i2;
            p0 = i1;
            i2 = 3u;
            i1 |= i2;
            i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
            i0 = p0;
            i1 = l3;
            i0 += i1;
            i1 = 4u;
            i0 += i1;
            p0 = i0;
            i1 = p0;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            i2 = 1u;
            i1 |= i2;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
          } else {
            i0 = l3;
            i1 = l2;
            i2 = 3u;
            i1 |= i2;
            i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
            i0 = l12;
            i1 = l8;
            i2 = 1u;
            i1 |= i2;
            i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
            i0 = l8;
            i1 = l12;
            i0 += i1;
            i1 = l8;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            i0 = l7;
            if (i0) {
              i0 = 4724u;
              i0 = i32_load(Z_envZ_memory, (u64)(i0));
              l4 = i0;
              i0 = l7;
              i1 = 3u;
              i0 >>= (i1 & 31);
              l1 = i0;
              i1 = 3u;
              i0 <<= (i1 & 31);
              i1 = 4744u;
              i0 += i1;
              p0 = i0;
              i0 = 1u;
              i1 = l1;
              i0 <<= (i1 & 31);
              l1 = i0;
              i1 = l5;
              i0 &= i1;
              if (i0) {
                i0 = p0;
                i1 = 8u;
                i0 += i1;
                l2 = i0;
                i0 = i32_load(Z_envZ_memory, (u64)(i0));
              } else {
                i0 = 4704u;
                i1 = l1;
                i2 = l5;
                i1 |= i2;
                i32_store(Z_envZ_memory, (u64)(i0), i1);
                i0 = p0;
                i1 = 8u;
                i0 += i1;
                l2 = i0;
                i0 = p0;
              }
              l1 = i0;
              i0 = l2;
              i1 = l4;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
              i0 = l1;
              i1 = l4;
              i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
              i0 = l4;
              i1 = l1;
              i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
              i0 = l4;
              i1 = p0;
              i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
            }
            i0 = 4712u;
            i1 = l8;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            i0 = 4724u;
            i1 = l12;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
          }
          i0 = l10;
          g7 = i0;
          i0 = l3;
          i1 = 8u;
          i0 += i1;
          goto Bfunc;
        } else {
          i0 = l2;
        }
      } else {
        i0 = l2;
      }
    } else {
      i0 = l2;
    }
  } else {
    i0 = p0;
    i1 = 4294967231u;
    i0 = i0 > i1;
    if (i0) {
      i0 = 4294967295u;
    } else {
      i0 = p0;
      i1 = 11u;
      i0 += i1;
      p0 = i0;
      i1 = 4294967288u;
      i0 &= i1;
      l1 = i0;
      i0 = 4708u;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l5 = i0;
      if (i0) {
        i0 = p0;
        i1 = 8u;
        i0 >>= (i1 & 31);
        p0 = i0;
        if (i0) {
          i0 = l1;
          i1 = 16777215u;
          i0 = i0 > i1;
          if (i0) {
            i0 = 31u;
          } else {
            i0 = 14u;
            i1 = p0;
            i2 = p0;
            i3 = 1048320u;
            i2 += i3;
            i3 = 16u;
            i2 >>= (i3 & 31);
            i3 = 8u;
            i2 &= i3;
            l2 = i2;
            i1 <<= (i2 & 31);
            l3 = i1;
            i2 = 520192u;
            i1 += i2;
            i2 = 16u;
            i1 >>= (i2 & 31);
            i2 = 4u;
            i1 &= i2;
            p0 = i1;
            i2 = l2;
            i1 |= i2;
            i2 = l3;
            i3 = p0;
            i2 <<= (i3 & 31);
            p0 = i2;
            i3 = 245760u;
            i2 += i3;
            i3 = 16u;
            i2 >>= (i3 & 31);
            i3 = 2u;
            i2 &= i3;
            l2 = i2;
            i1 |= i2;
            i0 -= i1;
            i1 = p0;
            i2 = l2;
            i1 <<= (i2 & 31);
            i2 = 15u;
            i1 >>= (i2 & 31);
            i0 += i1;
            p0 = i0;
            i1 = 1u;
            i0 <<= (i1 & 31);
            i1 = l1;
            i2 = p0;
            i3 = 7u;
            i2 += i3;
            i1 >>= (i2 & 31);
            i2 = 1u;
            i1 &= i2;
            i0 |= i1;
          }
        } else {
          i0 = 0u;
        }
        l7 = i0;
        i0 = 0u;
        i1 = l1;
        i0 -= i1;
        l3 = i0;
        i0 = l7;
        i1 = 2u;
        i0 <<= (i1 & 31);
        i1 = 5008u;
        i0 += i1;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        p0 = i0;
        if (i0) {
          i0 = 0u;
          l2 = i0;
          i0 = l1;
          i1 = 0u;
          i2 = 25u;
          i3 = l7;
          i4 = 1u;
          i3 >>= (i4 & 31);
          i2 -= i3;
          i3 = l7;
          i4 = 31u;
          i3 = i3 == i4;
          i1 = i3 ? i1 : i2;
          i0 <<= (i1 & 31);
          l6 = i0;
          L37: 
            i0 = p0;
            i0 = i32_load(Z_envZ_memory, (u64)(i0 + 4));
            i1 = 4294967288u;
            i0 &= i1;
            i1 = l1;
            i0 -= i1;
            l8 = i0;
            i1 = l3;
            i0 = i0 < i1;
            if (i0) {
              i0 = l8;
              if (i0) {
                i0 = l8;
                l3 = i0;
                i0 = p0;
              } else {
                i0 = p0;
                l2 = i0;
                i0 = 0u;
                l3 = i0;
                goto B35;
              }
              l2 = i0;
            }
            i0 = l4;
            i1 = p0;
            i1 = i32_load(Z_envZ_memory, (u64)(i1 + 20));
            l4 = i1;
            i2 = l4;
            i2 = !(i2);
            i3 = l4;
            i4 = p0;
            i5 = 16u;
            i4 += i5;
            i5 = l6;
            i6 = 31u;
            i5 >>= (i6 & 31);
            i6 = 2u;
            i5 <<= (i6 & 31);
            i4 += i5;
            i4 = i32_load(Z_envZ_memory, (u64)(i4));
            p0 = i4;
            i3 = i3 == i4;
            i2 |= i3;
            i0 = i2 ? i0 : i1;
            l4 = i0;
            i0 = l6;
            i1 = 1u;
            i0 <<= (i1 & 31);
            l6 = i0;
            i0 = p0;
            if (i0) {goto L37;}
            i0 = l2;
        } else {
          i0 = 0u;
        }
        p0 = i0;
        i1 = l4;
        i0 |= i1;
        i0 = !(i0);
        if (i0) {
          i0 = l1;
          i1 = l5;
          i2 = 2u;
          i3 = l7;
          i2 <<= (i3 & 31);
          p0 = i2;
          i3 = 0u;
          i4 = p0;
          i3 -= i4;
          i2 |= i3;
          i1 &= i2;
          l2 = i1;
          i1 = !(i1);
          if (i1) {goto B30;}
          i0 = l2;
          i1 = 0u;
          i2 = l2;
          i1 -= i2;
          i0 &= i1;
          i1 = 4294967295u;
          i0 += i1;
          l2 = i0;
          i1 = 12u;
          i0 >>= (i1 & 31);
          i1 = 16u;
          i0 &= i1;
          l4 = i0;
          i1 = l2;
          i2 = l4;
          i1 >>= (i2 & 31);
          l2 = i1;
          i2 = 5u;
          i1 >>= (i2 & 31);
          i2 = 8u;
          i1 &= i2;
          l4 = i1;
          i0 |= i1;
          i1 = l2;
          i2 = l4;
          i1 >>= (i2 & 31);
          l2 = i1;
          i2 = 2u;
          i1 >>= (i2 & 31);
          i2 = 4u;
          i1 &= i2;
          l4 = i1;
          i0 |= i1;
          i1 = l2;
          i2 = l4;
          i1 >>= (i2 & 31);
          l2 = i1;
          i2 = 1u;
          i1 >>= (i2 & 31);
          i2 = 2u;
          i1 &= i2;
          l4 = i1;
          i0 |= i1;
          i1 = l2;
          i2 = l4;
          i1 >>= (i2 & 31);
          l2 = i1;
          i2 = 1u;
          i1 >>= (i2 & 31);
          i2 = 1u;
          i1 &= i2;
          l4 = i1;
          i0 |= i1;
          i1 = l2;
          i2 = l4;
          i1 >>= (i2 & 31);
          i0 += i1;
          i1 = 2u;
          i0 <<= (i1 & 31);
          i1 = 5008u;
          i0 += i1;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l4 = i0;
          i0 = 0u;
          p0 = i0;
        }
        i0 = l4;
        if (i0) {
          i0 = p0;
          l2 = i0;
          i0 = l4;
          p0 = i0;
          goto B35;
        } else {
          i0 = p0;
        }
        l4 = i0;
        goto B34;
        B35:;
        i0 = l2;
        l4 = i0;
        i0 = l3;
        l2 = i0;
        L42: 
          i0 = p0;
          i0 = i32_load(Z_envZ_memory, (u64)(i0 + 4));
          i1 = 4294967288u;
          i0 &= i1;
          i1 = l1;
          i0 -= i1;
          l8 = i0;
          i1 = l2;
          i0 = i0 < i1;
          l6 = i0;
          i0 = l8;
          i1 = l2;
          i2 = l6;
          i0 = i2 ? i0 : i1;
          l2 = i0;
          i0 = p0;
          i1 = l4;
          i2 = l6;
          i0 = i2 ? i0 : i1;
          l4 = i0;
          i0 = p0;
          i0 = i32_load(Z_envZ_memory, (u64)(i0 + 16));
          l3 = i0;
          i0 = !(i0);
          if (i0) {
            i0 = p0;
            i0 = i32_load(Z_envZ_memory, (u64)(i0 + 20));
            l3 = i0;
          }
          i0 = l3;
          if (i0) {
            i0 = l3;
            p0 = i0;
            goto L42;
          } else {
            i0 = l2;
          }
        l3 = i0;
        B34:;
        i0 = l4;
        if (i0) {
          i0 = l3;
          i1 = 4712u;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i2 = l1;
          i1 -= i2;
          i0 = i0 < i1;
          if (i0) {
            i0 = l1;
            i1 = l4;
            i0 += i1;
            l7 = i0;
            i1 = l4;
            i0 = i0 > i1;
            if (i0) {
              i0 = l4;
              i0 = i32_load(Z_envZ_memory, (u64)(i0 + 24));
              l9 = i0;
              i0 = l4;
              i1 = l4;
              i1 = i32_load(Z_envZ_memory, (u64)(i1 + 12));
              p0 = i1;
              i0 = i0 == i1;
              if (i0) {
                i0 = l4;
                i1 = 20u;
                i0 += i1;
                l2 = i0;
                i0 = i32_load(Z_envZ_memory, (u64)(i0));
                p0 = i0;
                i0 = !(i0);
                if (i0) {
                  i0 = l4;
                  i1 = 16u;
                  i0 += i1;
                  l2 = i0;
                  i0 = i32_load(Z_envZ_memory, (u64)(i0));
                  p0 = i0;
                  i0 = !(i0);
                  if (i0) {
                    i0 = 0u;
                    p0 = i0;
                    goto B49;
                  }
                }
                L52: 
                  i0 = p0;
                  i1 = 20u;
                  i0 += i1;
                  l6 = i0;
                  i0 = i32_load(Z_envZ_memory, (u64)(i0));
                  l8 = i0;
                  if (i0) {
                    i0 = l6;
                    l2 = i0;
                    i0 = l8;
                  } else {
                    i0 = p0;
                    i1 = 16u;
                    i0 += i1;
                    l6 = i0;
                    i0 = i32_load(Z_envZ_memory, (u64)(i0));
                    l8 = i0;
                    i0 = !(i0);
                    if (i0) {goto B53;}
                    i0 = l6;
                    l2 = i0;
                    i0 = l8;
                  }
                  p0 = i0;
                  goto L52;
                  B53:;
                i0 = l2;
                i1 = 0u;
                i32_store(Z_envZ_memory, (u64)(i0), i1);
                B49:;
              } else {
                i0 = l4;
                i0 = i32_load(Z_envZ_memory, (u64)(i0 + 8));
                l2 = i0;
                i1 = p0;
                i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
                i0 = p0;
                i1 = l2;
                i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
              }
              i0 = l9;
              if (i0) {
                i0 = l4;
                i1 = l4;
                i1 = i32_load(Z_envZ_memory, (u64)(i1 + 28));
                l2 = i1;
                i2 = 2u;
                i1 <<= (i2 & 31);
                i2 = 5008u;
                i1 += i2;
                l6 = i1;
                i1 = i32_load(Z_envZ_memory, (u64)(i1));
                i0 = i0 == i1;
                if (i0) {
                  i0 = l6;
                  i1 = p0;
                  i32_store(Z_envZ_memory, (u64)(i0), i1);
                  i0 = p0;
                  i0 = !(i0);
                  if (i0) {
                    i0 = 4708u;
                    i1 = l5;
                    i2 = 1u;
                    i3 = l2;
                    i2 <<= (i3 & 31);
                    i3 = 4294967295u;
                    i2 ^= i3;
                    i1 &= i2;
                    p0 = i1;
                    i32_store(Z_envZ_memory, (u64)(i0), i1);
                    goto B56;
                  }
                } else {
                  i0 = l9;
                  i1 = 16u;
                  i0 += i1;
                  l2 = i0;
                  i1 = l9;
                  i2 = 20u;
                  i1 += i2;
                  i2 = l4;
                  i3 = l2;
                  i3 = i32_load(Z_envZ_memory, (u64)(i3));
                  i2 = i2 == i3;
                  i0 = i2 ? i0 : i1;
                  i1 = p0;
                  i32_store(Z_envZ_memory, (u64)(i0), i1);
                  i0 = p0;
                  i0 = !(i0);
                  if (i0) {
                    i0 = l5;
                    p0 = i0;
                    goto B56;
                  }
                }
                i0 = p0;
                i1 = l9;
                i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
                i0 = l4;
                i0 = i32_load(Z_envZ_memory, (u64)(i0 + 16));
                l2 = i0;
                if (i0) {
                  i0 = p0;
                  i1 = l2;
                  i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
                  i0 = l2;
                  i1 = p0;
                  i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
                }
                i0 = l4;
                i0 = i32_load(Z_envZ_memory, (u64)(i0 + 20));
                l2 = i0;
                if (i0) {
                  i0 = p0;
                  i1 = l2;
                  i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
                  i0 = l2;
                  i1 = p0;
                  i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
                  i0 = l5;
                } else {
                  i0 = l5;
                }
                p0 = i0;
                B56:;
              } else {
                i0 = l5;
                p0 = i0;
              }
              i0 = l3;
              i1 = 16u;
              i0 = i0 < i1;
              if (i0) {
                i0 = l4;
                i1 = l1;
                i2 = l3;
                i1 += i2;
                p0 = i1;
                i2 = 3u;
                i1 |= i2;
                i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
                i0 = p0;
                i1 = l4;
                i0 += i1;
                i1 = 4u;
                i0 += i1;
                p0 = i0;
                i1 = p0;
                i1 = i32_load(Z_envZ_memory, (u64)(i1));
                i2 = 1u;
                i1 |= i2;
                i32_store(Z_envZ_memory, (u64)(i0), i1);
              } else {
                i0 = l4;
                i1 = l1;
                i2 = 3u;
                i1 |= i2;
                i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
                i0 = l7;
                i1 = l3;
                i2 = 1u;
                i1 |= i2;
                i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
                i0 = l3;
                i1 = l7;
                i0 += i1;
                i1 = l3;
                i32_store(Z_envZ_memory, (u64)(i0), i1);
                i0 = l3;
                i1 = 3u;
                i0 >>= (i1 & 31);
                l1 = i0;
                i0 = l3;
                i1 = 256u;
                i0 = i0 < i1;
                if (i0) {
                  i0 = l1;
                  i1 = 3u;
                  i0 <<= (i1 & 31);
                  i1 = 4744u;
                  i0 += i1;
                  p0 = i0;
                  i0 = 4704u;
                  i0 = i32_load(Z_envZ_memory, (u64)(i0));
                  l2 = i0;
                  i1 = 1u;
                  i2 = l1;
                  i1 <<= (i2 & 31);
                  l1 = i1;
                  i0 &= i1;
                  if (i0) {
                    i0 = p0;
                    i1 = 8u;
                    i0 += i1;
                    l2 = i0;
                    i0 = i32_load(Z_envZ_memory, (u64)(i0));
                  } else {
                    i0 = 4704u;
                    i1 = l1;
                    i2 = l2;
                    i1 |= i2;
                    i32_store(Z_envZ_memory, (u64)(i0), i1);
                    i0 = p0;
                    i1 = 8u;
                    i0 += i1;
                    l2 = i0;
                    i0 = p0;
                  }
                  l1 = i0;
                  i0 = l2;
                  i1 = l7;
                  i32_store(Z_envZ_memory, (u64)(i0), i1);
                  i0 = l1;
                  i1 = l7;
                  i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
                  i0 = l7;
                  i1 = l1;
                  i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
                  i0 = l7;
                  i1 = p0;
                  i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
                  goto B63;
                }
                i0 = l3;
                i1 = 8u;
                i0 >>= (i1 & 31);
                l1 = i0;
                if (i0) {
                  i0 = l3;
                  i1 = 16777215u;
                  i0 = i0 > i1;
                  if (i0) {
                    i0 = 31u;
                  } else {
                    i0 = 14u;
                    i1 = l1;
                    i2 = l1;
                    i3 = 1048320u;
                    i2 += i3;
                    i3 = 16u;
                    i2 >>= (i3 & 31);
                    i3 = 8u;
                    i2 &= i3;
                    l2 = i2;
                    i1 <<= (i2 & 31);
                    l5 = i1;
                    i2 = 520192u;
                    i1 += i2;
                    i2 = 16u;
                    i1 >>= (i2 & 31);
                    i2 = 4u;
                    i1 &= i2;
                    l1 = i1;
                    i2 = l2;
                    i1 |= i2;
                    i2 = l5;
                    i3 = l1;
                    i2 <<= (i3 & 31);
                    l1 = i2;
                    i3 = 245760u;
                    i2 += i3;
                    i3 = 16u;
                    i2 >>= (i3 & 31);
                    i3 = 2u;
                    i2 &= i3;
                    l2 = i2;
                    i1 |= i2;
                    i0 -= i1;
                    i1 = l1;
                    i2 = l2;
                    i1 <<= (i2 & 31);
                    i2 = 15u;
                    i1 >>= (i2 & 31);
                    i0 += i1;
                    l1 = i0;
                    i1 = 1u;
                    i0 <<= (i1 & 31);
                    i1 = l3;
                    i2 = l1;
                    i3 = 7u;
                    i2 += i3;
                    i1 >>= (i2 & 31);
                    i2 = 1u;
                    i1 &= i2;
                    i0 |= i1;
                  }
                } else {
                  i0 = 0u;
                }
                l1 = i0;
                i1 = 2u;
                i0 <<= (i1 & 31);
                i1 = 5008u;
                i0 += i1;
                l2 = i0;
                i0 = l7;
                i1 = l1;
                i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
                i0 = l7;
                i1 = 16u;
                i0 += i1;
                l5 = i0;
                i1 = 0u;
                i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
                i0 = l5;
                i1 = 0u;
                i32_store(Z_envZ_memory, (u64)(i0), i1);
                i0 = 1u;
                i1 = l1;
                i0 <<= (i1 & 31);
                l5 = i0;
                i1 = p0;
                i0 &= i1;
                i0 = !(i0);
                if (i0) {
                  i0 = 4708u;
                  i1 = p0;
                  i2 = l5;
                  i1 |= i2;
                  i32_store(Z_envZ_memory, (u64)(i0), i1);
                  i0 = l2;
                  i1 = l7;
                  i32_store(Z_envZ_memory, (u64)(i0), i1);
                  i0 = l7;
                  i1 = l2;
                  i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
                  i0 = l7;
                  i1 = l7;
                  i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
                  i0 = l7;
                  i1 = l7;
                  i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
                  goto B63;
                }
                i0 = l3;
                i1 = l2;
                i1 = i32_load(Z_envZ_memory, (u64)(i1));
                p0 = i1;
                i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
                i2 = 4294967288u;
                i1 &= i2;
                i0 = i0 == i1;
                if (i0) {
                  i0 = p0;
                  l1 = i0;
                } else {
                  i0 = l3;
                  i1 = 0u;
                  i2 = 25u;
                  i3 = l1;
                  i4 = 1u;
                  i3 >>= (i4 & 31);
                  i2 -= i3;
                  i3 = l1;
                  i4 = 31u;
                  i3 = i3 == i4;
                  i1 = i3 ? i1 : i2;
                  i0 <<= (i1 & 31);
                  l2 = i0;
                  L71: 
                    i0 = p0;
                    i1 = 16u;
                    i0 += i1;
                    i1 = l2;
                    i2 = 31u;
                    i1 >>= (i2 & 31);
                    i2 = 2u;
                    i1 <<= (i2 & 31);
                    i0 += i1;
                    l5 = i0;
                    i0 = i32_load(Z_envZ_memory, (u64)(i0));
                    l1 = i0;
                    if (i0) {
                      i0 = l2;
                      i1 = 1u;
                      i0 <<= (i1 & 31);
                      l2 = i0;
                      i0 = l3;
                      i1 = l1;
                      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
                      i2 = 4294967288u;
                      i1 &= i2;
                      i0 = i0 == i1;
                      if (i0) {goto B70;}
                      i0 = l1;
                      p0 = i0;
                      goto L71;
                    }
                  i0 = l5;
                  i1 = l7;
                  i32_store(Z_envZ_memory, (u64)(i0), i1);
                  i0 = l7;
                  i1 = p0;
                  i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
                  i0 = l7;
                  i1 = l7;
                  i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
                  i0 = l7;
                  i1 = l7;
                  i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
                  goto B63;
                  B70:;
                }
                i0 = l1;
                i1 = 8u;
                i0 += i1;
                p0 = i0;
                i0 = i32_load(Z_envZ_memory, (u64)(i0));
                l2 = i0;
                i1 = l7;
                i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
                i0 = p0;
                i1 = l7;
                i32_store(Z_envZ_memory, (u64)(i0), i1);
                i0 = l7;
                i1 = l2;
                i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
                i0 = l7;
                i1 = l1;
                i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
                i0 = l7;
                i1 = 0u;
                i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
                B63:;
              }
              i0 = l10;
              g7 = i0;
              i0 = l4;
              i1 = 8u;
              i0 += i1;
              goto Bfunc;
            } else {
              i0 = l1;
            }
          } else {
            i0 = l1;
          }
        } else {
          i0 = l1;
        }
      } else {
        i0 = l1;
      }
      B30:;
    }
  }
  p0 = i0;
  i0 = 4712u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l2 = i0;
  i1 = p0;
  i0 = i0 >= i1;
  if (i0) {
    i0 = 4724u;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l1 = i0;
    i0 = l2;
    i1 = p0;
    i0 -= i1;
    l3 = i0;
    i1 = 15u;
    i0 = i0 > i1;
    if (i0) {
      i0 = 4724u;
      i1 = p0;
      i2 = l1;
      i1 += i2;
      l5 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4712u;
      i1 = l3;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l5;
      i1 = l3;
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
      i0 = l1;
      i1 = l2;
      i0 += i1;
      i1 = l3;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l1;
      i1 = p0;
      i2 = 3u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
    } else {
      i0 = 4712u;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4724u;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l1;
      i1 = l2;
      i2 = 3u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
      i0 = l1;
      i1 = l2;
      i0 += i1;
      i1 = 4u;
      i0 += i1;
      p0 = i0;
      i1 = p0;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    i0 = l10;
    g7 = i0;
    i0 = l1;
    i1 = 8u;
    i0 += i1;
    goto Bfunc;
  }
  i0 = 4716u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l2 = i0;
  i1 = p0;
  i0 = i0 > i1;
  if (i0) {
    i0 = 4716u;
    i1 = l2;
    i2 = p0;
    i1 -= i2;
    l2 = i1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4728u;
    i1 = p0;
    i2 = 4728u;
    i2 = i32_load(Z_envZ_memory, (u64)(i2));
    l1 = i2;
    i1 += i2;
    l3 = i1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = l2;
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
    i0 = l1;
    i1 = p0;
    i2 = 3u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
    i0 = l10;
    g7 = i0;
    i0 = l1;
    i1 = 8u;
    i0 += i1;
    goto Bfunc;
  }
  i0 = l10;
  l1 = i0;
  i0 = p0;
  i1 = 47u;
  i0 += i1;
  l4 = i0;
  i1 = 5176u;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  if (i1) {
    i1 = 5184u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
  } else {
    i1 = 5184u;
    i2 = 4096u;
    i32_store(Z_envZ_memory, (u64)(i1), i2);
    i1 = 5180u;
    i2 = 4096u;
    i32_store(Z_envZ_memory, (u64)(i1), i2);
    i1 = 5188u;
    i2 = 4294967295u;
    i32_store(Z_envZ_memory, (u64)(i1), i2);
    i1 = 5192u;
    i2 = 4294967295u;
    i32_store(Z_envZ_memory, (u64)(i1), i2);
    i1 = 5196u;
    i2 = 0u;
    i32_store(Z_envZ_memory, (u64)(i1), i2);
    i1 = 5148u;
    i2 = 0u;
    i32_store(Z_envZ_memory, (u64)(i1), i2);
    i1 = 5176u;
    i2 = l1;
    i3 = 4294967280u;
    i2 &= i3;
    i3 = 1431655768u;
    i2 ^= i3;
    i32_store(Z_envZ_memory, (u64)(i1), i2);
    i1 = 4096u;
  }
  l1 = i1;
  i0 += i1;
  l6 = i0;
  i1 = 0u;
  i2 = l1;
  i1 -= i2;
  l8 = i1;
  i0 &= i1;
  l5 = i0;
  i1 = p0;
  i0 = i0 <= i1;
  if (i0) {
    i0 = l10;
    g7 = i0;
    i0 = 0u;
    goto Bfunc;
  }
  i0 = 5144u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  if (i0) {
    i0 = l5;
    i1 = 5136u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    l3 = i1;
    i0 += i1;
    l7 = i0;
    i1 = l3;
    i0 = i0 <= i1;
    i1 = l7;
    i2 = l1;
    i1 = i1 > i2;
    i0 |= i1;
    if (i0) {
      i0 = l10;
      g7 = i0;
      i0 = 0u;
      goto Bfunc;
    }
  }
  i0 = p0;
  i1 = 48u;
  i0 += i1;
  l7 = i0;
  i0 = 5148u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  i1 = 4u;
  i0 &= i1;
  if (i0) {
    i0 = 0u;
    l2 = i0;
  } else {
    i0 = 4728u;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l1 = i0;
    i0 = !(i0);
    if (i0) {goto B85;}
    i0 = 5152u;
    l3 = i0;
    L86: 
      i0 = l3;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l9 = i0;
      i1 = l1;
      i0 = i0 <= i1;
      if (i0) {
        i0 = l9;
        i1 = l3;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
        i0 += i1;
        i1 = l1;
        i0 = i0 > i1;
        if (i0) {goto B87;}
      }
      i0 = l3;
      i0 = i32_load(Z_envZ_memory, (u64)(i0 + 8));
      l3 = i0;
      if (i0) {goto L86;}
      goto B85;
      B87:;
    i0 = l8;
    i1 = l6;
    i2 = l2;
    i1 -= i2;
    i0 &= i1;
    l2 = i0;
    i1 = 2147483647u;
    i0 = i0 < i1;
    if (i0) {
      i0 = l2;
      i0 = _sbrk(i0);
      l1 = i0;
      i1 = l3;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = l3;
      i2 = i32_load(Z_envZ_memory, (u64)(i2 + 4));
      i1 += i2;
      i0 = i0 == i1;
      if (i0) {
        i0 = l1;
        i1 = 4294967295u;
        i0 = i0 != i1;
        if (i0) {goto B81;}
      } else {
        goto B84;
      }
    } else {
      i0 = 0u;
      l2 = i0;
    }
    goto B83;
    B85:;
    i0 = 0u;
    i0 = _sbrk(i0);
    l1 = i0;
    i1 = 4294967295u;
    i0 = i0 == i1;
    if (i0) {
      i0 = 0u;
    } else {
      i0 = 5136u;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l6 = i0;
      i1 = l5;
      i2 = l1;
      i3 = 5180u;
      i3 = i32_load(Z_envZ_memory, (u64)(i3));
      l2 = i3;
      i4 = 4294967295u;
      i3 += i4;
      l3 = i3;
      i2 += i3;
      i3 = 0u;
      i4 = l2;
      i3 -= i4;
      i2 &= i3;
      i3 = l1;
      i2 -= i3;
      i3 = 0u;
      i4 = l1;
      i5 = l3;
      i4 &= i5;
      i2 = i4 ? i2 : i3;
      i1 += i2;
      l2 = i1;
      i0 += i1;
      l3 = i0;
      i0 = l2;
      i1 = 2147483647u;
      i0 = i0 < i1;
      i1 = l2;
      i2 = p0;
      i1 = i1 > i2;
      i0 &= i1;
      if (i0) {
        i0 = 5144u;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l8 = i0;
        if (i0) {
          i0 = l3;
          i1 = l6;
          i0 = i0 <= i1;
          i1 = l3;
          i2 = l8;
          i1 = i1 > i2;
          i0 |= i1;
          if (i0) {
            i0 = 0u;
            l2 = i0;
            goto B83;
          }
        }
        i0 = l1;
        i1 = l2;
        i1 = _sbrk(i1);
        l3 = i1;
        i0 = i0 == i1;
        if (i0) {goto B81;}
        i0 = l3;
        l1 = i0;
        goto B84;
      } else {
        i0 = 0u;
      }
    }
    l2 = i0;
    goto B83;
    B84:;
    i0 = l1;
    i1 = 4294967295u;
    i0 = i0 != i1;
    i1 = l2;
    i2 = 2147483647u;
    i1 = i1 < i2;
    i0 &= i1;
    i1 = l7;
    i2 = l2;
    i1 = i1 > i2;
    i0 &= i1;
    i0 = !(i0);
    if (i0) {
      i0 = l1;
      i1 = 4294967295u;
      i0 = i0 == i1;
      if (i0) {
        i0 = 0u;
        l2 = i0;
        goto B83;
      } else {
        goto B81;
      }
      UNREACHABLE;
    }
    i0 = 5184u;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i1 = l4;
    i2 = l2;
    i1 -= i2;
    i0 += i1;
    i1 = 0u;
    i2 = l3;
    i1 -= i2;
    i0 &= i1;
    l3 = i0;
    i1 = 2147483647u;
    i0 = i0 >= i1;
    if (i0) {goto B81;}
    i0 = 0u;
    i1 = l2;
    i0 -= i1;
    l4 = i0;
    i0 = l3;
    i0 = _sbrk(i0);
    i1 = 4294967295u;
    i0 = i0 == i1;
    if (i0) {
      i0 = l4;
      i0 = _sbrk(i0);
      i0 = 0u;
    } else {
      i0 = l2;
      i1 = l3;
      i0 += i1;
      l2 = i0;
      goto B81;
    }
    l2 = i0;
    B83:;
    i0 = 5148u;
    i1 = 5148u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 4u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  }
  i0 = l5;
  i1 = 2147483647u;
  i0 = i0 < i1;
  if (i0) {
    i0 = l5;
    i0 = _sbrk(i0);
    l1 = i0;
    i0 = 0u;
    i0 = _sbrk(i0);
    l3 = i0;
    i1 = l1;
    i0 -= i1;
    l4 = i0;
    i1 = p0;
    i2 = 40u;
    i1 += i2;
    i0 = i0 > i1;
    l5 = i0;
    i0 = l4;
    i1 = l2;
    i2 = l5;
    i0 = i2 ? i0 : i1;
    l2 = i0;
    i0 = l5;
    i1 = 1u;
    i0 ^= i1;
    i1 = l1;
    i2 = 4294967295u;
    i1 = i1 == i2;
    i0 |= i1;
    i1 = l1;
    i2 = 4294967295u;
    i1 = i1 != i2;
    i2 = l3;
    i3 = 4294967295u;
    i2 = i2 != i3;
    i1 &= i2;
    i2 = l1;
    i3 = l3;
    i2 = i2 < i3;
    i1 &= i2;
    i2 = 1u;
    i1 ^= i2;
    i0 |= i1;
    i0 = !(i0);
    if (i0) {goto B81;}
  }
  goto B80;
  B81:;
  i0 = 5136u;
  i1 = l2;
  i2 = 5136u;
  i2 = i32_load(Z_envZ_memory, (u64)(i2));
  i1 += i2;
  l3 = i1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 5140u;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i0 = i0 > i1;
  if (i0) {
    i0 = 5140u;
    i1 = l3;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  }
  i0 = 4728u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l5 = i0;
  if (i0) {
    i0 = 5152u;
    l3 = i0;
    L104: 
      i0 = l1;
      i1 = l3;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      l4 = i1;
      i2 = l3;
      i2 = i32_load(Z_envZ_memory, (u64)(i2 + 4));
      l6 = i2;
      i1 += i2;
      i0 = i0 == i1;
      if (i0) {goto B103;}
      i0 = l3;
      i0 = i32_load(Z_envZ_memory, (u64)(i0 + 8));
      l3 = i0;
      if (i0) {goto L104;}
    goto B102;
    B103:;
    i0 = l3;
    i1 = 4u;
    i0 += i1;
    l8 = i0;
    i0 = l3;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 12));
    i1 = 8u;
    i0 &= i1;
    i0 = !(i0);
    if (i0) {
      i0 = l4;
      i1 = l5;
      i0 = i0 <= i1;
      i1 = l1;
      i2 = l5;
      i1 = i1 > i2;
      i0 &= i1;
      if (i0) {
        i0 = l8;
        i1 = l2;
        i2 = l6;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l5;
        i1 = 0u;
        i2 = l5;
        i3 = 8u;
        i2 += i3;
        l1 = i2;
        i1 -= i2;
        i2 = 7u;
        i1 &= i2;
        i2 = 0u;
        i3 = l1;
        i4 = 7u;
        i3 &= i4;
        i1 = i3 ? i1 : i2;
        l3 = i1;
        i0 += i1;
        l1 = i0;
        i0 = l2;
        i1 = 4716u;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i0 += i1;
        l4 = i0;
        i1 = l3;
        i0 -= i1;
        l2 = i0;
        i0 = 4728u;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = 4716u;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l1;
        i1 = l2;
        i2 = 1u;
        i1 |= i2;
        i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
        i0 = l4;
        i1 = l5;
        i0 += i1;
        i1 = 40u;
        i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
        i0 = 4732u;
        i1 = 5192u;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        goto B101;
      }
    }
    B102:;
    i0 = l1;
    i1 = 4720u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 < i1;
    if (i0) {
      i0 = 4720u;
      i1 = l1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    i0 = l1;
    i1 = l2;
    i0 += i1;
    l4 = i0;
    i0 = 5152u;
    l3 = i0;
    L110: 
      i0 = l4;
      i1 = l3;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i0 = i0 == i1;
      if (i0) {goto B109;}
      i0 = l3;
      i0 = i32_load(Z_envZ_memory, (u64)(i0 + 8));
      l3 = i0;
      if (i0) {goto L110;}
    goto B108;
    B109:;
    i0 = l3;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 12));
    i1 = 8u;
    i0 &= i1;
    i0 = !(i0);
    if (i0) {
      i0 = l3;
      i1 = l1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i1 = 4u;
      i0 += i1;
      l3 = i0;
      i1 = l2;
      i2 = l3;
      i2 = i32_load(Z_envZ_memory, (u64)(i2));
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = l1;
      i2 = 0u;
      i3 = l1;
      i4 = 8u;
      i3 += i4;
      l1 = i3;
      i2 -= i3;
      i3 = 7u;
      i2 &= i3;
      i3 = 0u;
      i4 = l1;
      i5 = 7u;
      i4 &= i5;
      i2 = i4 ? i2 : i3;
      i1 += i2;
      l7 = i1;
      i0 += i1;
      l6 = i0;
      i0 = l4;
      i1 = 0u;
      i2 = l4;
      i3 = 8u;
      i2 += i3;
      l1 = i2;
      i1 -= i2;
      i2 = 7u;
      i1 &= i2;
      i2 = 0u;
      i3 = l1;
      i4 = 7u;
      i3 &= i4;
      i1 = i3 ? i1 : i2;
      i0 += i1;
      l2 = i0;
      i1 = l7;
      i0 -= i1;
      i1 = p0;
      i0 -= i1;
      l3 = i0;
      i0 = l7;
      i1 = p0;
      i2 = 3u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
      i0 = l2;
      i1 = l5;
      i0 = i0 == i1;
      if (i0) {
        i0 = 4716u;
        i1 = l3;
        i2 = 4716u;
        i2 = i32_load(Z_envZ_memory, (u64)(i2));
        i1 += i2;
        p0 = i1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = 4728u;
        i1 = l6;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l6;
        i1 = p0;
        i2 = 1u;
        i1 |= i2;
        i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
      } else {
        i0 = l2;
        i1 = 4724u;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i0 = i0 == i1;
        if (i0) {
          i0 = 4712u;
          i1 = l3;
          i2 = 4712u;
          i2 = i32_load(Z_envZ_memory, (u64)(i2));
          i1 += i2;
          p0 = i1;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = 4724u;
          i1 = l6;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l6;
          i1 = p0;
          i2 = 1u;
          i1 |= i2;
          i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
          i0 = p0;
          i1 = l6;
          i0 += i1;
          i1 = p0;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          goto B113;
        }
        i0 = l2;
        i0 = i32_load(Z_envZ_memory, (u64)(i0 + 4));
        l9 = i0;
        i1 = 3u;
        i0 &= i1;
        i1 = 1u;
        i0 = i0 == i1;
        if (i0) {
          i0 = l9;
          i1 = 3u;
          i0 >>= (i1 & 31);
          l5 = i0;
          i0 = l9;
          i1 = 256u;
          i0 = i0 < i1;
          if (i0) {
            i0 = l2;
            i0 = i32_load(Z_envZ_memory, (u64)(i0 + 8));
            p0 = i0;
            i1 = l2;
            i1 = i32_load(Z_envZ_memory, (u64)(i1 + 12));
            l1 = i1;
            i0 = i0 == i1;
            if (i0) {
              i0 = 4704u;
              i1 = 4704u;
              i1 = i32_load(Z_envZ_memory, (u64)(i1));
              i2 = 1u;
              i3 = l5;
              i2 <<= (i3 & 31);
              i3 = 4294967295u;
              i2 ^= i3;
              i1 &= i2;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
            } else {
              i0 = p0;
              i1 = l1;
              i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
              i0 = l1;
              i1 = p0;
              i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
            }
          } else {
            i0 = l2;
            i0 = i32_load(Z_envZ_memory, (u64)(i0 + 24));
            l8 = i0;
            i0 = l2;
            i1 = l2;
            i1 = i32_load(Z_envZ_memory, (u64)(i1 + 12));
            p0 = i1;
            i0 = i0 == i1;
            if (i0) {
              i0 = l2;
              i1 = 16u;
              i0 += i1;
              l1 = i0;
              i1 = 4u;
              i0 += i1;
              l5 = i0;
              i0 = i32_load(Z_envZ_memory, (u64)(i0));
              p0 = i0;
              if (i0) {
                i0 = l5;
                l1 = i0;
              } else {
                i0 = l1;
                i0 = i32_load(Z_envZ_memory, (u64)(i0));
                p0 = i0;
                i0 = !(i0);
                if (i0) {
                  i0 = 0u;
                  p0 = i0;
                  goto B120;
                }
              }
              L123: 
                i0 = p0;
                i1 = 20u;
                i0 += i1;
                l5 = i0;
                i0 = i32_load(Z_envZ_memory, (u64)(i0));
                l4 = i0;
                if (i0) {
                  i0 = l5;
                  l1 = i0;
                  i0 = l4;
                } else {
                  i0 = p0;
                  i1 = 16u;
                  i0 += i1;
                  l5 = i0;
                  i0 = i32_load(Z_envZ_memory, (u64)(i0));
                  l4 = i0;
                  i0 = !(i0);
                  if (i0) {goto B124;}
                  i0 = l5;
                  l1 = i0;
                  i0 = l4;
                }
                p0 = i0;
                goto L123;
                B124:;
              i0 = l1;
              i1 = 0u;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
              B120:;
            } else {
              i0 = l2;
              i0 = i32_load(Z_envZ_memory, (u64)(i0 + 8));
              l1 = i0;
              i1 = p0;
              i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
              i0 = p0;
              i1 = l1;
              i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
            }
            i0 = l8;
            i0 = !(i0);
            if (i0) {goto B118;}
            i0 = l2;
            i1 = l2;
            i1 = i32_load(Z_envZ_memory, (u64)(i1 + 28));
            l1 = i1;
            i2 = 2u;
            i1 <<= (i2 & 31);
            i2 = 5008u;
            i1 += i2;
            l5 = i1;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            i0 = i0 == i1;
            if (i0) {
              i0 = l5;
              i1 = p0;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
              i0 = p0;
              if (i0) {goto B127;}
              i0 = 4708u;
              i1 = 4708u;
              i1 = i32_load(Z_envZ_memory, (u64)(i1));
              i2 = 1u;
              i3 = l1;
              i2 <<= (i3 & 31);
              i3 = 4294967295u;
              i2 ^= i3;
              i1 &= i2;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
              goto B118;
              B127:;
            } else {
              i0 = l8;
              i1 = 16u;
              i0 += i1;
              l1 = i0;
              i1 = l8;
              i2 = 20u;
              i1 += i2;
              i2 = l2;
              i3 = l1;
              i3 = i32_load(Z_envZ_memory, (u64)(i3));
              i2 = i2 == i3;
              i0 = i2 ? i0 : i1;
              i1 = p0;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
              i0 = p0;
              i0 = !(i0);
              if (i0) {goto B118;}
            }
            i0 = p0;
            i1 = l8;
            i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
            i0 = l2;
            i1 = 16u;
            i0 += i1;
            l5 = i0;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
            l1 = i0;
            if (i0) {
              i0 = p0;
              i1 = l1;
              i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
              i0 = l1;
              i1 = p0;
              i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
            }
            i0 = l5;
            i0 = i32_load(Z_envZ_memory, (u64)(i0 + 4));
            l1 = i0;
            i0 = !(i0);
            if (i0) {goto B118;}
            i0 = p0;
            i1 = l1;
            i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
            i0 = l1;
            i1 = p0;
            i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
            B118:;
          }
          i0 = l2;
          i1 = l9;
          i2 = 4294967288u;
          i1 &= i2;
          p0 = i1;
          i0 += i1;
          l2 = i0;
          i0 = p0;
          i1 = l3;
          i0 += i1;
          l3 = i0;
        }
        i0 = l2;
        i1 = 4u;
        i0 += i1;
        p0 = i0;
        i1 = p0;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i2 = 4294967294u;
        i1 &= i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l6;
        i1 = l3;
        i2 = 1u;
        i1 |= i2;
        i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
        i0 = l3;
        i1 = l6;
        i0 += i1;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        i1 = 3u;
        i0 >>= (i1 & 31);
        l1 = i0;
        i0 = l3;
        i1 = 256u;
        i0 = i0 < i1;
        if (i0) {
          i0 = l1;
          i1 = 3u;
          i0 <<= (i1 & 31);
          i1 = 4744u;
          i0 += i1;
          p0 = i0;
          i0 = 4704u;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l2 = i0;
          i1 = 1u;
          i2 = l1;
          i1 <<= (i2 & 31);
          l1 = i1;
          i0 &= i1;
          if (i0) {
            i0 = p0;
            i1 = 8u;
            i0 += i1;
            l2 = i0;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
          } else {
            i0 = 4704u;
            i1 = l1;
            i2 = l2;
            i1 |= i2;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            i0 = p0;
            i1 = 8u;
            i0 += i1;
            l2 = i0;
            i0 = p0;
          }
          l1 = i0;
          i0 = l2;
          i1 = l6;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l1;
          i1 = l6;
          i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
          i0 = l6;
          i1 = l1;
          i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
          i0 = l6;
          i1 = p0;
          i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
          goto B113;
        }
        i0 = l3;
        i1 = 8u;
        i0 >>= (i1 & 31);
        p0 = i0;
        if (i0) {
          i0 = l3;
          i1 = 16777215u;
          i0 = i0 > i1;
          if (i0) {
            i0 = 31u;
          } else {
            i0 = 14u;
            i1 = p0;
            i2 = p0;
            i3 = 1048320u;
            i2 += i3;
            i3 = 16u;
            i2 >>= (i3 & 31);
            i3 = 8u;
            i2 &= i3;
            l1 = i2;
            i1 <<= (i2 & 31);
            l2 = i1;
            i2 = 520192u;
            i1 += i2;
            i2 = 16u;
            i1 >>= (i2 & 31);
            i2 = 4u;
            i1 &= i2;
            p0 = i1;
            i2 = l1;
            i1 |= i2;
            i2 = l2;
            i3 = p0;
            i2 <<= (i3 & 31);
            p0 = i2;
            i3 = 245760u;
            i2 += i3;
            i3 = 16u;
            i2 >>= (i3 & 31);
            i3 = 2u;
            i2 &= i3;
            l1 = i2;
            i1 |= i2;
            i0 -= i1;
            i1 = p0;
            i2 = l1;
            i1 <<= (i2 & 31);
            i2 = 15u;
            i1 >>= (i2 & 31);
            i0 += i1;
            p0 = i0;
            i1 = 1u;
            i0 <<= (i1 & 31);
            i1 = l3;
            i2 = p0;
            i3 = 7u;
            i2 += i3;
            i1 >>= (i2 & 31);
            i2 = 1u;
            i1 &= i2;
            i0 |= i1;
          }
        } else {
          i0 = 0u;
        }
        l1 = i0;
        i1 = 2u;
        i0 <<= (i1 & 31);
        i1 = 5008u;
        i0 += i1;
        p0 = i0;
        i0 = l6;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
        i0 = l6;
        i1 = 16u;
        i0 += i1;
        l2 = i0;
        i1 = 0u;
        i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
        i0 = l2;
        i1 = 0u;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = 4708u;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l2 = i0;
        i1 = 1u;
        i2 = l1;
        i1 <<= (i2 & 31);
        l5 = i1;
        i0 &= i1;
        i0 = !(i0);
        if (i0) {
          i0 = 4708u;
          i1 = l2;
          i2 = l5;
          i1 |= i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = p0;
          i1 = l6;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l6;
          i1 = p0;
          i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
          i0 = l6;
          i1 = l6;
          i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
          i0 = l6;
          i1 = l6;
          i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
          goto B113;
        }
        i0 = l3;
        i1 = p0;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        p0 = i1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
        i2 = 4294967288u;
        i1 &= i2;
        i0 = i0 == i1;
        if (i0) {
          i0 = p0;
          l1 = i0;
        } else {
          i0 = l3;
          i1 = 0u;
          i2 = 25u;
          i3 = l1;
          i4 = 1u;
          i3 >>= (i4 & 31);
          i2 -= i3;
          i3 = l1;
          i4 = 31u;
          i3 = i3 == i4;
          i1 = i3 ? i1 : i2;
          i0 <<= (i1 & 31);
          l2 = i0;
          L136: 
            i0 = p0;
            i1 = 16u;
            i0 += i1;
            i1 = l2;
            i2 = 31u;
            i1 >>= (i2 & 31);
            i2 = 2u;
            i1 <<= (i2 & 31);
            i0 += i1;
            l5 = i0;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
            l1 = i0;
            if (i0) {
              i0 = l2;
              i1 = 1u;
              i0 <<= (i1 & 31);
              l2 = i0;
              i0 = l3;
              i1 = l1;
              i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
              i2 = 4294967288u;
              i1 &= i2;
              i0 = i0 == i1;
              if (i0) {goto B135;}
              i0 = l1;
              p0 = i0;
              goto L136;
            }
          i0 = l5;
          i1 = l6;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l6;
          i1 = p0;
          i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
          i0 = l6;
          i1 = l6;
          i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
          i0 = l6;
          i1 = l6;
          i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
          goto B113;
          B135:;
        }
        i0 = l1;
        i1 = 8u;
        i0 += i1;
        p0 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l2 = i0;
        i1 = l6;
        i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
        i0 = p0;
        i1 = l6;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l6;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
        i0 = l6;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
        i0 = l6;
        i1 = 0u;
        i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
        B113:;
      }
      i0 = l10;
      g7 = i0;
      i0 = l7;
      i1 = 8u;
      i0 += i1;
      goto Bfunc;
    }
    B108:;
    i0 = 5152u;
    l3 = i0;
    L138: 
      i0 = l3;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l4 = i0;
      i1 = l5;
      i0 = i0 <= i1;
      if (i0) {
        i0 = l4;
        i1 = l3;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
        i0 += i1;
        l6 = i0;
        i1 = l5;
        i0 = i0 > i1;
        if (i0) {goto B139;}
      }
      i0 = l3;
      i0 = i32_load(Z_envZ_memory, (u64)(i0 + 8));
      l3 = i0;
      goto L138;
      B139:;
    i0 = l5;
    i1 = 0u;
    i2 = l6;
    i3 = 4294967249u;
    i2 += i3;
    l4 = i2;
    i3 = 8u;
    i2 += i3;
    l3 = i2;
    i1 -= i2;
    i2 = 7u;
    i1 &= i2;
    i2 = 0u;
    i3 = l3;
    i4 = 7u;
    i3 &= i4;
    i1 = i3 ? i1 : i2;
    i2 = l4;
    i1 += i2;
    l3 = i1;
    i2 = l3;
    i3 = l5;
    i4 = 16u;
    i3 += i4;
    l7 = i3;
    i2 = i2 < i3;
    i0 = i2 ? i0 : i1;
    l3 = i0;
    i1 = 8u;
    i0 += i1;
    l4 = i0;
    i0 = 4728u;
    i1 = l1;
    i2 = 0u;
    i3 = l1;
    i4 = 8u;
    i3 += i4;
    l8 = i3;
    i2 -= i3;
    i3 = 7u;
    i2 &= i3;
    i3 = 0u;
    i4 = l8;
    i5 = 7u;
    i4 &= i5;
    i2 = i4 ? i2 : i3;
    l8 = i2;
    i1 += i2;
    l9 = i1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4716u;
    i1 = l2;
    i2 = 4294967256u;
    i1 += i2;
    l11 = i1;
    i2 = l8;
    i1 -= i2;
    l8 = i1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l9;
    i1 = l8;
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
    i0 = l1;
    i1 = l11;
    i0 += i1;
    i1 = 40u;
    i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
    i0 = 4732u;
    i1 = 5192u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 4u;
    i0 += i1;
    l8 = i0;
    i1 = 27u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l4;
    i1 = 5152u;
    j1 = i64_load(Z_envZ_memory, (u64)(i1));
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    i0 = l4;
    i1 = 5160u;
    j1 = i64_load(Z_envZ_memory, (u64)(i1));
    i64_store(Z_envZ_memory, (u64)(i0 + 8), j1);
    i0 = 5152u;
    i1 = l1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 5156u;
    i1 = l2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 5164u;
    i1 = 0u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 5160u;
    i1 = l4;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 24u;
    i0 += i1;
    l1 = i0;
    L141: 
      i0 = l1;
      i1 = 4u;
      i0 += i1;
      l2 = i0;
      i1 = 7u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l1;
      i1 = 8u;
      i0 += i1;
      i1 = l6;
      i0 = i0 < i1;
      if (i0) {
        i0 = l2;
        l1 = i0;
        goto L141;
      }
    i0 = l3;
    i1 = l5;
    i0 = i0 != i1;
    if (i0) {
      i0 = l8;
      i1 = l8;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 4294967294u;
      i1 &= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l5;
      i1 = l3;
      i2 = l5;
      i1 -= i2;
      l4 = i1;
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
      i0 = l3;
      i1 = l4;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l4;
      i1 = 3u;
      i0 >>= (i1 & 31);
      l2 = i0;
      i0 = l4;
      i1 = 256u;
      i0 = i0 < i1;
      if (i0) {
        i0 = l2;
        i1 = 3u;
        i0 <<= (i1 & 31);
        i1 = 4744u;
        i0 += i1;
        l1 = i0;
        i0 = 4704u;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l3 = i0;
        i1 = 1u;
        i2 = l2;
        i1 <<= (i2 & 31);
        l2 = i1;
        i0 &= i1;
        if (i0) {
          i0 = l1;
          i1 = 8u;
          i0 += i1;
          l3 = i0;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
        } else {
          i0 = 4704u;
          i1 = l2;
          i2 = l3;
          i1 |= i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l1;
          i1 = 8u;
          i0 += i1;
          l3 = i0;
          i0 = l1;
        }
        l2 = i0;
        i0 = l3;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l2;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
        i0 = l5;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
        i0 = l5;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
        goto B101;
      }
      i0 = l4;
      i1 = 8u;
      i0 >>= (i1 & 31);
      l1 = i0;
      if (i0) {
        i0 = l4;
        i1 = 16777215u;
        i0 = i0 > i1;
        if (i0) {
          i0 = 31u;
        } else {
          i0 = 14u;
          i1 = l1;
          i2 = l1;
          i3 = 1048320u;
          i2 += i3;
          i3 = 16u;
          i2 >>= (i3 & 31);
          i3 = 8u;
          i2 &= i3;
          l2 = i2;
          i1 <<= (i2 & 31);
          l3 = i1;
          i2 = 520192u;
          i1 += i2;
          i2 = 16u;
          i1 >>= (i2 & 31);
          i2 = 4u;
          i1 &= i2;
          l1 = i1;
          i2 = l2;
          i1 |= i2;
          i2 = l3;
          i3 = l1;
          i2 <<= (i3 & 31);
          l1 = i2;
          i3 = 245760u;
          i2 += i3;
          i3 = 16u;
          i2 >>= (i3 & 31);
          i3 = 2u;
          i2 &= i3;
          l2 = i2;
          i1 |= i2;
          i0 -= i1;
          i1 = l1;
          i2 = l2;
          i1 <<= (i2 & 31);
          i2 = 15u;
          i1 >>= (i2 & 31);
          i0 += i1;
          l1 = i0;
          i1 = 1u;
          i0 <<= (i1 & 31);
          i1 = l4;
          i2 = l1;
          i3 = 7u;
          i2 += i3;
          i1 >>= (i2 & 31);
          i2 = 1u;
          i1 &= i2;
          i0 |= i1;
        }
      } else {
        i0 = 0u;
      }
      l2 = i0;
      i1 = 2u;
      i0 <<= (i1 & 31);
      i1 = 5008u;
      i0 += i1;
      l1 = i0;
      i0 = l5;
      i1 = l2;
      i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
      i0 = l5;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
      i0 = l7;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4708u;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      i1 = 1u;
      i2 = l2;
      i1 <<= (i2 & 31);
      l6 = i1;
      i0 &= i1;
      i0 = !(i0);
      if (i0) {
        i0 = 4708u;
        i1 = l3;
        i2 = l6;
        i1 |= i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l1;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l5;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
        i0 = l5;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
        i0 = l5;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
        goto B101;
      }
      i0 = l4;
      i1 = l1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      l1 = i1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
      i2 = 4294967288u;
      i1 &= i2;
      i0 = i0 == i1;
      if (i0) {
        i0 = l1;
        l2 = i0;
      } else {
        i0 = l4;
        i1 = 0u;
        i2 = 25u;
        i3 = l2;
        i4 = 1u;
        i3 >>= (i4 & 31);
        i2 -= i3;
        i3 = l2;
        i4 = 31u;
        i3 = i3 == i4;
        i1 = i3 ? i1 : i2;
        i0 <<= (i1 & 31);
        l3 = i0;
        L151: 
          i0 = l1;
          i1 = 16u;
          i0 += i1;
          i1 = l3;
          i2 = 31u;
          i1 >>= (i2 & 31);
          i2 = 2u;
          i1 <<= (i2 & 31);
          i0 += i1;
          l6 = i0;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l2 = i0;
          if (i0) {
            i0 = l3;
            i1 = 1u;
            i0 <<= (i1 & 31);
            l3 = i0;
            i0 = l4;
            i1 = l2;
            i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
            i2 = 4294967288u;
            i1 &= i2;
            i0 = i0 == i1;
            if (i0) {goto B150;}
            i0 = l2;
            l1 = i0;
            goto L151;
          }
        i0 = l6;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l5;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
        i0 = l5;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
        i0 = l5;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
        goto B101;
        B150:;
      }
      i0 = l2;
      i1 = 8u;
      i0 += i1;
      l1 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      i1 = l5;
      i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
      i0 = l1;
      i1 = l5;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l5;
      i1 = l3;
      i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
      i0 = l5;
      i1 = l2;
      i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
      i0 = l5;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
    }
    B101:;
  } else {
    i0 = 4720u;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i0 = !(i0);
    i1 = l1;
    i2 = l3;
    i1 = i1 < i2;
    i0 |= i1;
    if (i0) {
      i0 = 4720u;
      i1 = l1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    i0 = 5152u;
    i1 = l1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 5156u;
    i1 = l2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 5164u;
    i1 = 0u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4740u;
    i1 = 5176u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4736u;
    i1 = 4294967295u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4756u;
    i1 = 4744u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4752u;
    i1 = 4744u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4764u;
    i1 = 4752u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4760u;
    i1 = 4752u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4772u;
    i1 = 4760u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4768u;
    i1 = 4760u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4780u;
    i1 = 4768u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4776u;
    i1 = 4768u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4788u;
    i1 = 4776u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4784u;
    i1 = 4776u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4796u;
    i1 = 4784u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4792u;
    i1 = 4784u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4804u;
    i1 = 4792u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4800u;
    i1 = 4792u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4812u;
    i1 = 4800u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4808u;
    i1 = 4800u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4820u;
    i1 = 4808u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4816u;
    i1 = 4808u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4828u;
    i1 = 4816u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4824u;
    i1 = 4816u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4836u;
    i1 = 4824u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4832u;
    i1 = 4824u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4844u;
    i1 = 4832u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4840u;
    i1 = 4832u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4852u;
    i1 = 4840u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4848u;
    i1 = 4840u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4860u;
    i1 = 4848u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4856u;
    i1 = 4848u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4868u;
    i1 = 4856u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4864u;
    i1 = 4856u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4876u;
    i1 = 4864u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4872u;
    i1 = 4864u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4884u;
    i1 = 4872u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4880u;
    i1 = 4872u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4892u;
    i1 = 4880u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4888u;
    i1 = 4880u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4900u;
    i1 = 4888u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4896u;
    i1 = 4888u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4908u;
    i1 = 4896u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4904u;
    i1 = 4896u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4916u;
    i1 = 4904u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4912u;
    i1 = 4904u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4924u;
    i1 = 4912u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4920u;
    i1 = 4912u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4932u;
    i1 = 4920u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4928u;
    i1 = 4920u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4940u;
    i1 = 4928u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4936u;
    i1 = 4928u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4948u;
    i1 = 4936u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4944u;
    i1 = 4936u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4956u;
    i1 = 4944u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4952u;
    i1 = 4944u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4964u;
    i1 = 4952u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4960u;
    i1 = 4952u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4972u;
    i1 = 4960u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4968u;
    i1 = 4960u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4980u;
    i1 = 4968u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4976u;
    i1 = 4968u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4988u;
    i1 = 4976u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4984u;
    i1 = 4976u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4996u;
    i1 = 4984u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4992u;
    i1 = 4984u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 5004u;
    i1 = 4992u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 5000u;
    i1 = 4992u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4728u;
    i1 = l1;
    i2 = 0u;
    i3 = l1;
    i4 = 8u;
    i3 += i4;
    l3 = i3;
    i2 -= i3;
    i3 = 7u;
    i2 &= i3;
    i3 = 0u;
    i4 = l3;
    i5 = 7u;
    i4 &= i5;
    i2 = i4 ? i2 : i3;
    l3 = i2;
    i1 += i2;
    l5 = i1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4716u;
    i1 = l2;
    i2 = 4294967256u;
    i1 += i2;
    l2 = i1;
    i2 = l3;
    i1 -= i2;
    l3 = i1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l5;
    i1 = l3;
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
    i0 = l1;
    i1 = l2;
    i0 += i1;
    i1 = 40u;
    i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
    i0 = 4732u;
    i1 = 5192u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  }
  i0 = 4716u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i1 = p0;
  i0 = i0 > i1;
  if (i0) {
    i0 = 4716u;
    i1 = l1;
    i2 = p0;
    i1 -= i2;
    l2 = i1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4728u;
    i1 = p0;
    i2 = 4728u;
    i2 = i32_load(Z_envZ_memory, (u64)(i2));
    l1 = i2;
    i1 += i2;
    l3 = i1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = l2;
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
    i0 = l1;
    i1 = p0;
    i2 = 3u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
    i0 = l10;
    g7 = i0;
    i0 = l1;
    i1 = 8u;
    i0 += i1;
    goto Bfunc;
  }
  B80:;
  i0 = 5200u;
  i1 = 12u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l10;
  g7 = i0;
  i0 = 0u;
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static void _free(u32 p0) {
  u32 l1 = 0, l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0, 
      l9 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  i0 = p0;
  i0 = !(i0);
  if (i0) {
    goto Bfunc;
  }
  i0 = 4720u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l4 = i0;
  i0 = p0;
  i1 = 4294967288u;
  i0 += i1;
  l3 = i0;
  i1 = p0;
  i2 = 4294967292u;
  i1 += i2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  l2 = i1;
  i2 = 4294967288u;
  i1 &= i2;
  p0 = i1;
  i0 += i1;
  l5 = i0;
  i0 = l2;
  i1 = 1u;
  i0 &= i1;
  if (i0) {
    i0 = l3;
  } else {
    i0 = l3;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l1 = i0;
    i0 = l2;
    i1 = 3u;
    i0 &= i1;
    i0 = !(i0);
    if (i0) {
      goto Bfunc;
    }
    i0 = l3;
    i1 = l1;
    i0 -= i1;
    l3 = i0;
    i1 = l4;
    i0 = i0 < i1;
    if (i0) {
      goto Bfunc;
    }
    i0 = p0;
    i1 = l1;
    i0 += i1;
    p0 = i0;
    i0 = l3;
    i1 = 4724u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 == i1;
    if (i0) {
      i0 = l3;
      i1 = l5;
      i2 = 4u;
      i1 += i2;
      l1 = i1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      l2 = i1;
      i2 = 3u;
      i1 &= i2;
      i2 = 3u;
      i1 = i1 != i2;
      if (i1) {goto B2;}
      i0 = 4712u;
      i1 = p0;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l1;
      i1 = l2;
      i2 = 4294967294u;
      i1 &= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i1 = 4u;
      i0 += i1;
      i1 = p0;
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = l3;
      i0 += i1;
      i1 = p0;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      goto Bfunc;
    }
    i0 = l1;
    i1 = 3u;
    i0 >>= (i1 & 31);
    l4 = i0;
    i0 = l1;
    i1 = 256u;
    i0 = i0 < i1;
    if (i0) {
      i0 = l3;
      i1 = 8u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l1 = i0;
      i1 = l3;
      i2 = 12u;
      i1 += i2;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      l2 = i1;
      i0 = i0 == i1;
      if (i0) {
        i0 = 4704u;
        i1 = 4704u;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i2 = 1u;
        i3 = l4;
        i2 <<= (i3 & 31);
        i3 = 4294967295u;
        i2 ^= i3;
        i1 &= i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        goto B2;
      } else {
        i0 = l1;
        i1 = 12u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l2;
        i1 = 8u;
        i0 += i1;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        goto B2;
      }
      UNREACHABLE;
    }
    i0 = l3;
    i1 = 24u;
    i0 += i1;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l7 = i0;
    i0 = l3;
    i1 = l3;
    i2 = 12u;
    i1 += i2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    l1 = i1;
    i0 = i0 == i1;
    if (i0) {
      i0 = l3;
      i1 = 16u;
      i0 += i1;
      l2 = i0;
      i1 = 4u;
      i0 += i1;
      l4 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l1 = i0;
      if (i0) {
        i0 = l4;
        l2 = i0;
      } else {
        i0 = l2;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l1 = i0;
        i0 = !(i0);
        if (i0) {
          i0 = 0u;
          l1 = i0;
          goto B9;
        }
      }
      L12: 
        i0 = l1;
        i1 = 20u;
        i0 += i1;
        l4 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l6 = i0;
        if (i0) {
          i0 = l4;
          l2 = i0;
          i0 = l6;
        } else {
          i0 = l1;
          i1 = 16u;
          i0 += i1;
          l4 = i0;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l6 = i0;
          i0 = !(i0);
          if (i0) {goto B13;}
          i0 = l4;
          l2 = i0;
          i0 = l6;
        }
        l1 = i0;
        goto L12;
        B13:;
      i0 = l2;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      B9:;
    } else {
      i0 = l3;
      i1 = 8u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l2 = i0;
      i1 = 12u;
      i0 += i1;
      i1 = l1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l1;
      i1 = 8u;
      i0 += i1;
      i1 = l2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    i0 = l7;
    if (i0) {
      i0 = l3;
      i1 = l3;
      i2 = 28u;
      i1 += i2;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      l2 = i1;
      i2 = 2u;
      i1 <<= (i2 & 31);
      i2 = 5008u;
      i1 += i2;
      l4 = i1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i0 = i0 == i1;
      if (i0) {
        i0 = l4;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l1;
        i0 = !(i0);
        if (i0) {
          i0 = 4708u;
          i1 = 4708u;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i2 = 1u;
          i3 = l2;
          i2 <<= (i3 & 31);
          i3 = 4294967295u;
          i2 ^= i3;
          i1 &= i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l3;
          goto B2;
        }
      } else {
        i0 = l7;
        i1 = 16u;
        i0 += i1;
        l2 = i0;
        i1 = l7;
        i2 = 20u;
        i1 += i2;
        i2 = l3;
        i3 = l2;
        i3 = i32_load(Z_envZ_memory, (u64)(i3));
        i2 = i2 == i3;
        i0 = i2 ? i0 : i1;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        i1 = l1;
        i1 = !(i1);
        if (i1) {goto B2;}
      }
      i0 = l1;
      i1 = 24u;
      i0 += i1;
      i1 = l7;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i1 = 16u;
      i0 += i1;
      l4 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l2 = i0;
      if (i0) {
        i0 = l1;
        i1 = 16u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l2;
        i1 = 24u;
        i0 += i1;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
      i0 = l4;
      i1 = 4u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l2 = i0;
      if (i0) {
        i0 = l1;
        i1 = 20u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l2;
        i1 = 24u;
        i0 += i1;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
      } else {
        i0 = l3;
      }
    } else {
      i0 = l3;
    }
    B2:;
  }
  l7 = i0;
  i1 = l5;
  i0 = i0 >= i1;
  if (i0) {
    goto Bfunc;
  }
  i0 = l5;
  i1 = 4u;
  i0 += i1;
  l1 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l8 = i0;
  i1 = 1u;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {
    goto Bfunc;
  }
  i0 = l8;
  i1 = 2u;
  i0 &= i1;
  if (i0) {
    i0 = l1;
    i1 = l8;
    i2 = 4294967294u;
    i1 &= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 4u;
    i0 += i1;
    i1 = p0;
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l7;
    i0 += i1;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    l2 = i0;
  } else {
    i0 = l5;
    i1 = 4728u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 == i1;
    if (i0) {
      i0 = 4716u;
      i1 = p0;
      i2 = 4716u;
      i2 = i32_load(Z_envZ_memory, (u64)(i2));
      i1 += i2;
      p0 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4728u;
      i1 = l3;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i1 = 4u;
      i0 += i1;
      i1 = p0;
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4724u;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      i1 = l3;
      i0 = i0 != i1;
      if (i0) {
        goto Bfunc;
      }
      i0 = 4724u;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4712u;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      goto Bfunc;
    }
    i0 = 4724u;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = l5;
    i0 = i0 == i1;
    if (i0) {
      i0 = 4712u;
      i1 = p0;
      i2 = 4712u;
      i2 = i32_load(Z_envZ_memory, (u64)(i2));
      i1 += i2;
      p0 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4724u;
      i1 = l7;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i1 = 4u;
      i0 += i1;
      i1 = p0;
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = l7;
      i0 += i1;
      i1 = p0;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      goto Bfunc;
    }
    i0 = l8;
    i1 = 3u;
    i0 >>= (i1 & 31);
    l4 = i0;
    i0 = l8;
    i1 = 256u;
    i0 = i0 < i1;
    if (i0) {
      i0 = l5;
      i1 = 8u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l1 = i0;
      i1 = l5;
      i2 = 12u;
      i1 += i2;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      l2 = i1;
      i0 = i0 == i1;
      if (i0) {
        i0 = 4704u;
        i1 = 4704u;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i2 = 1u;
        i3 = l4;
        i2 <<= (i3 & 31);
        i3 = 4294967295u;
        i2 ^= i3;
        i1 &= i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      } else {
        i0 = l1;
        i1 = 12u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l2;
        i1 = 8u;
        i0 += i1;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
    } else {
      i0 = l5;
      i1 = 24u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l9 = i0;
      i0 = l5;
      i1 = 12u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l1 = i0;
      i1 = l5;
      i0 = i0 == i1;
      if (i0) {
        i0 = l5;
        i1 = 16u;
        i0 += i1;
        l2 = i0;
        i1 = 4u;
        i0 += i1;
        l4 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l1 = i0;
        if (i0) {
          i0 = l4;
          l2 = i0;
        } else {
          i0 = l2;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l1 = i0;
          i0 = !(i0);
          if (i0) {
            i0 = 0u;
            l1 = i0;
            goto B30;
          }
        }
        L33: 
          i0 = l1;
          i1 = 20u;
          i0 += i1;
          l4 = i0;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l6 = i0;
          if (i0) {
            i0 = l4;
            l2 = i0;
            i0 = l6;
          } else {
            i0 = l1;
            i1 = 16u;
            i0 += i1;
            l4 = i0;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
            l6 = i0;
            i0 = !(i0);
            if (i0) {goto B34;}
            i0 = l4;
            l2 = i0;
            i0 = l6;
          }
          l1 = i0;
          goto L33;
          B34:;
        i0 = l2;
        i1 = 0u;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        B30:;
      } else {
        i0 = l5;
        i1 = 8u;
        i0 += i1;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l2 = i0;
        i1 = 12u;
        i0 += i1;
        i1 = l1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l1;
        i1 = 8u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
      i0 = l9;
      if (i0) {
        i0 = l5;
        i1 = 28u;
        i0 += i1;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l2 = i0;
        i1 = 2u;
        i0 <<= (i1 & 31);
        i1 = 5008u;
        i0 += i1;
        l4 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        i1 = l5;
        i0 = i0 == i1;
        if (i0) {
          i0 = l4;
          i1 = l1;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l1;
          i0 = !(i0);
          if (i0) {
            i0 = 4708u;
            i1 = 4708u;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            i2 = 1u;
            i3 = l2;
            i2 <<= (i3 & 31);
            i3 = 4294967295u;
            i2 ^= i3;
            i1 &= i2;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            goto B28;
          }
        } else {
          i0 = l9;
          i1 = 16u;
          i0 += i1;
          l2 = i0;
          i1 = l9;
          i2 = 20u;
          i1 += i2;
          i2 = l2;
          i2 = i32_load(Z_envZ_memory, (u64)(i2));
          i3 = l5;
          i2 = i2 == i3;
          i0 = i2 ? i0 : i1;
          i1 = l1;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l1;
          i0 = !(i0);
          if (i0) {goto B28;}
        }
        i0 = l1;
        i1 = 24u;
        i0 += i1;
        i1 = l9;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l5;
        i1 = 16u;
        i0 += i1;
        l4 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l2 = i0;
        if (i0) {
          i0 = l1;
          i1 = 16u;
          i0 += i1;
          i1 = l2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l2;
          i1 = 24u;
          i0 += i1;
          i1 = l1;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
        }
        i0 = l4;
        i1 = 4u;
        i0 += i1;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l2 = i0;
        if (i0) {
          i0 = l1;
          i1 = 20u;
          i0 += i1;
          i1 = l2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l2;
          i1 = 24u;
          i0 += i1;
          i1 = l1;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
        }
      }
      B28:;
    }
    i0 = l3;
    i1 = 4u;
    i0 += i1;
    i1 = p0;
    i2 = l8;
    i3 = 4294967288u;
    i2 &= i3;
    i1 += i2;
    l2 = i1;
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l2;
    i1 = l7;
    i0 += i1;
    i1 = l2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 4724u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 == i1;
    if (i0) {
      i0 = 4712u;
      i1 = l2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      goto Bfunc;
    }
  }
  i0 = l2;
  i1 = 3u;
  i0 >>= (i1 & 31);
  l1 = i0;
  i0 = l2;
  i1 = 256u;
  i0 = i0 < i1;
  if (i0) {
    i0 = l1;
    i1 = 3u;
    i0 <<= (i1 & 31);
    i1 = 4744u;
    i0 += i1;
    p0 = i0;
    i0 = 4704u;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l2 = i0;
    i1 = 1u;
    i2 = l1;
    i1 <<= (i2 & 31);
    l1 = i1;
    i0 &= i1;
    if (i0) {
      i0 = p0;
      i1 = 8u;
      i0 += i1;
      l2 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
    } else {
      i0 = 4704u;
      i1 = l1;
      i2 = l2;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = 8u;
      i0 += i1;
      l2 = i0;
      i0 = p0;
    }
    l1 = i0;
    i0 = l2;
    i1 = l3;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l1;
    i1 = 12u;
    i0 += i1;
    i1 = l3;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 8u;
    i0 += i1;
    i1 = l1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 12u;
    i0 += i1;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    goto Bfunc;
  }
  i0 = l2;
  i1 = 8u;
  i0 >>= (i1 & 31);
  p0 = i0;
  if (i0) {
    i0 = l2;
    i1 = 16777215u;
    i0 = i0 > i1;
    if (i0) {
      i0 = 31u;
    } else {
      i0 = p0;
      i1 = p0;
      i2 = 1048320u;
      i1 += i2;
      i2 = 16u;
      i1 >>= (i2 & 31);
      i2 = 8u;
      i1 &= i2;
      l1 = i1;
      i0 <<= (i1 & 31);
      l4 = i0;
      i1 = 520192u;
      i0 += i1;
      i1 = 16u;
      i0 >>= (i1 & 31);
      i1 = 4u;
      i0 &= i1;
      p0 = i0;
      i0 = 14u;
      i1 = p0;
      i2 = l1;
      i1 |= i2;
      i2 = l4;
      i3 = p0;
      i2 <<= (i3 & 31);
      p0 = i2;
      i3 = 245760u;
      i2 += i3;
      i3 = 16u;
      i2 >>= (i3 & 31);
      i3 = 2u;
      i2 &= i3;
      l1 = i2;
      i1 |= i2;
      i0 -= i1;
      i1 = p0;
      i2 = l1;
      i1 <<= (i2 & 31);
      i2 = 15u;
      i1 >>= (i2 & 31);
      i0 += i1;
      p0 = i0;
      i1 = 1u;
      i0 <<= (i1 & 31);
      i1 = l2;
      i2 = p0;
      i3 = 7u;
      i2 += i3;
      i1 >>= (i2 & 31);
      i2 = 1u;
      i1 &= i2;
      i0 |= i1;
    }
  } else {
    i0 = 0u;
  }
  l1 = i0;
  i1 = 2u;
  i0 <<= (i1 & 31);
  i1 = 5008u;
  i0 += i1;
  p0 = i0;
  i0 = l3;
  i1 = 28u;
  i0 += i1;
  i1 = l1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 20u;
  i0 += i1;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 16u;
  i0 += i1;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 4708u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l4 = i0;
  i1 = 1u;
  i2 = l1;
  i1 <<= (i2 & 31);
  l6 = i1;
  i0 &= i1;
  if (i0) {
    i0 = l2;
    i1 = p0;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    p0 = i1;
    i2 = 4u;
    i1 += i2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 4294967288u;
    i1 &= i2;
    i0 = i0 == i1;
    if (i0) {
      i0 = p0;
      l1 = i0;
    } else {
      i0 = l2;
      i1 = 0u;
      i2 = 25u;
      i3 = l1;
      i4 = 1u;
      i3 >>= (i4 & 31);
      i2 -= i3;
      i3 = l1;
      i4 = 31u;
      i3 = i3 == i4;
      i1 = i3 ? i1 : i2;
      i0 <<= (i1 & 31);
      l4 = i0;
      L50: 
        i0 = p0;
        i1 = 16u;
        i0 += i1;
        i1 = l4;
        i2 = 31u;
        i1 >>= (i2 & 31);
        i2 = 2u;
        i1 <<= (i2 & 31);
        i0 += i1;
        l6 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l1 = i0;
        if (i0) {
          i0 = l4;
          i1 = 1u;
          i0 <<= (i1 & 31);
          l4 = i0;
          i0 = l2;
          i1 = l1;
          i2 = 4u;
          i1 += i2;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i2 = 4294967288u;
          i1 &= i2;
          i0 = i0 == i1;
          if (i0) {goto B49;}
          i0 = l1;
          p0 = i0;
          goto L50;
        }
      i0 = l6;
      i1 = l3;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i1 = 24u;
      i0 += i1;
      i1 = p0;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i1 = 12u;
      i0 += i1;
      i1 = l3;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i1 = 8u;
      i0 += i1;
      i1 = l3;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      goto B47;
      B49:;
    }
    i0 = l1;
    i1 = 8u;
    i0 += i1;
    p0 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l2 = i0;
    i1 = 12u;
    i0 += i1;
    i1 = l3;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l3;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 8u;
    i0 += i1;
    i1 = l2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 12u;
    i0 += i1;
    i1 = l1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 24u;
    i0 += i1;
    i1 = 0u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    B47:;
  } else {
    i0 = 4708u;
    i1 = l4;
    i2 = l6;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l3;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 24u;
    i0 += i1;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 12u;
    i0 += i1;
    i1 = l3;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 8u;
    i0 += i1;
    i1 = l3;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  }
  i0 = 4736u;
  i1 = 4736u;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i2 = 4294967295u;
  i1 += i2;
  p0 = i1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  if (i0) {
    goto Bfunc;
  }
  i0 = 5160u;
  p0 = i0;
  L53: 
    i0 = p0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i1 = 8u;
    i0 += i1;
    p0 = i0;
    i0 = l3;
    if (i0) {goto L53;}
  i0 = 4736u;
  i1 = 4294967295u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  Bfunc:;
  FUNC_EPILOGUE;
}

static u32 f19(u32 p0, u32 p1) {
  u32 l2 = 0, l3 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5, i6;
  i0 = p0;
  i0 = !(i0);
  if (i0) {
    i0 = p1;
    i0 = _malloc(i0);
    goto Bfunc;
  }
  i0 = p1;
  i1 = 4294967231u;
  i0 = i0 > i1;
  if (i0) {
    i0 = 5200u;
    i1 = 12u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 0u;
    goto Bfunc;
  }
  i0 = p0;
  i1 = 4294967288u;
  i0 += i1;
  i1 = 16u;
  i2 = p1;
  i3 = 11u;
  i2 += i3;
  i3 = 4294967288u;
  i2 &= i3;
  i3 = p1;
  i4 = 11u;
  i3 = i3 < i4;
  i1 = i3 ? i1 : i2;
  i0 = f20(i0, i1);
  l2 = i0;
  if (i0) {
    i0 = l2;
    i1 = 8u;
    i0 += i1;
    goto Bfunc;
  }
  i0 = p1;
  i0 = _malloc(i0);
  l2 = i0;
  i0 = !(i0);
  if (i0) {
    i0 = 0u;
    goto Bfunc;
  }
  i0 = l2;
  i1 = p0;
  i2 = p0;
  i3 = 4294967292u;
  i2 += i3;
  i2 = i32_load(Z_envZ_memory, (u64)(i2));
  l3 = i2;
  i3 = 4294967288u;
  i2 &= i3;
  i3 = 4u;
  i4 = 8u;
  i5 = l3;
  i6 = 3u;
  i5 &= i6;
  i3 = i5 ? i3 : i4;
  i2 -= i3;
  l3 = i2;
  i3 = p1;
  i4 = l3;
  i5 = p1;
  i4 = i4 < i5;
  i2 = i4 ? i2 : i3;
  i0 = _memcpy(i0, i1, i2);
  i0 = p0;
  _free(i0);
  i0 = l2;
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f20(u32 p0, u32 p1) {
  u32 l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0, l9 = 0, 
      l10 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  i0 = p0;
  i1 = 4u;
  i0 += i1;
  l7 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l6 = i0;
  i1 = 4294967288u;
  i0 &= i1;
  l2 = i0;
  i0 = l6;
  i1 = 3u;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {
    i0 = p1;
    i1 = 256u;
    i0 = i0 < i1;
    if (i0) {
      i0 = 0u;
      goto Bfunc;
    }
    i0 = l2;
    i1 = p1;
    i2 = 4u;
    i1 += i2;
    i0 = i0 >= i1;
    if (i0) {
      i0 = l2;
      i1 = p1;
      i0 -= i1;
      i1 = 5184u;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 1u;
      i1 <<= (i2 & 31);
      i0 = i0 <= i1;
      if (i0) {
        i0 = p0;
        goto Bfunc;
      }
    }
    i0 = 0u;
    goto Bfunc;
  }
  i0 = p0;
  i1 = l2;
  i0 += i1;
  l4 = i0;
  i0 = l2;
  i1 = p1;
  i0 = i0 >= i1;
  if (i0) {
    i0 = l2;
    i1 = p1;
    i0 -= i1;
    l2 = i0;
    i1 = 15u;
    i0 = i0 <= i1;
    if (i0) {
      i0 = p0;
      goto Bfunc;
    }
    i0 = l7;
    i1 = p1;
    i2 = l6;
    i3 = 1u;
    i2 &= i3;
    i1 |= i2;
    i2 = 2u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = p1;
    i0 += i1;
    p1 = i0;
    i1 = 4u;
    i0 += i1;
    i1 = l2;
    i2 = 3u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l4;
    i1 = 4u;
    i0 += i1;
    l3 = i0;
    i1 = l3;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    i1 = l2;
    f21(i0, i1);
    i0 = p0;
    goto Bfunc;
  }
  i0 = 4728u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  i1 = l4;
  i0 = i0 == i1;
  if (i0) {
    i0 = 4716u;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = l2;
    i0 += i1;
    l2 = i0;
    i1 = p1;
    i0 = i0 <= i1;
    if (i0) {
      i0 = 0u;
      goto Bfunc;
    }
    i0 = l7;
    i1 = p1;
    i2 = l6;
    i3 = 1u;
    i2 &= i3;
    i1 |= i2;
    i2 = 2u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = p1;
    i0 += i1;
    l3 = i0;
    i1 = 4u;
    i0 += i1;
    i1 = l2;
    i2 = p1;
    i1 -= i2;
    p1 = i1;
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4728u;
    i1 = l3;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4716u;
    i1 = p1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    goto Bfunc;
  }
  i0 = 4724u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  i1 = l4;
  i0 = i0 == i1;
  if (i0) {
    i0 = l2;
    i1 = 4712u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 += i1;
    l3 = i0;
    i1 = p1;
    i0 = i0 < i1;
    if (i0) {
      i0 = 0u;
      goto Bfunc;
    }
    i0 = l3;
    i1 = p1;
    i0 -= i1;
    l2 = i0;
    i1 = 15u;
    i0 = i0 > i1;
    if (i0) {
      i0 = l7;
      i1 = p1;
      i2 = l6;
      i3 = 1u;
      i2 &= i3;
      i1 |= i2;
      i2 = 2u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = p1;
      i0 += i1;
      p1 = i0;
      i1 = 4u;
      i0 += i1;
      i1 = l2;
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = l3;
      i0 += i1;
      l3 = i0;
      i1 = l2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i1 = 4u;
      i0 += i1;
      l3 = i0;
      i1 = l3;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 4294967294u;
      i1 &= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    } else {
      i0 = l7;
      i1 = l3;
      i2 = l6;
      i3 = 1u;
      i2 &= i3;
      i1 |= i2;
      i2 = 2u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = l3;
      i0 += i1;
      i1 = 4u;
      i0 += i1;
      p1 = i0;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 0u;
      p1 = i0;
      i0 = 0u;
      l2 = i0;
    }
    i0 = 4712u;
    i1 = l2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4724u;
    i1 = p1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    goto Bfunc;
  }
  i0 = l4;
  i1 = 4u;
  i0 += i1;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l3 = i0;
  i1 = 2u;
  i0 &= i1;
  if (i0) {
    i0 = 0u;
    goto Bfunc;
  }
  i0 = l2;
  i1 = l3;
  i2 = 4294967288u;
  i1 &= i2;
  i0 += i1;
  l8 = i0;
  i1 = p1;
  i0 = i0 < i1;
  if (i0) {
    i0 = 0u;
    goto Bfunc;
  }
  i0 = l3;
  i1 = 3u;
  i0 >>= (i1 & 31);
  l5 = i0;
  i0 = l3;
  i1 = 256u;
  i0 = i0 < i1;
  if (i0) {
    i0 = l4;
    i1 = 8u;
    i0 += i1;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l2 = i0;
    i1 = l4;
    i2 = 12u;
    i1 += i2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    l3 = i1;
    i0 = i0 == i1;
    if (i0) {
      i0 = 4704u;
      i1 = 4704u;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 1u;
      i3 = l5;
      i2 <<= (i3 & 31);
      i3 = 4294967295u;
      i2 ^= i3;
      i1 &= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    } else {
      i0 = l2;
      i1 = 12u;
      i0 += i1;
      i1 = l3;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i1 = 8u;
      i0 += i1;
      i1 = l2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
  } else {
    i0 = l4;
    i1 = 24u;
    i0 += i1;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l9 = i0;
    i0 = l4;
    i1 = l4;
    i2 = 12u;
    i1 += i2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    l2 = i1;
    i0 = i0 == i1;
    if (i0) {
      i0 = l4;
      i1 = 16u;
      i0 += i1;
      l3 = i0;
      i1 = 4u;
      i0 += i1;
      l5 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l2 = i0;
      if (i0) {
        i0 = l5;
        l3 = i0;
      } else {
        i0 = l3;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l2 = i0;
        i0 = !(i0);
        if (i0) {
          i0 = 0u;
          l2 = i0;
          goto B17;
        }
      }
      L20: 
        i0 = l2;
        i1 = 20u;
        i0 += i1;
        l5 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l10 = i0;
        if (i0) {
          i0 = l5;
          l3 = i0;
          i0 = l10;
        } else {
          i0 = l2;
          i1 = 16u;
          i0 += i1;
          l5 = i0;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l10 = i0;
          i0 = !(i0);
          if (i0) {goto B21;}
          i0 = l5;
          l3 = i0;
          i0 = l10;
        }
        l2 = i0;
        goto L20;
        B21:;
      i0 = l3;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      B17:;
    } else {
      i0 = l4;
      i1 = 8u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      i1 = 12u;
      i0 += i1;
      i1 = l2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l2;
      i1 = 8u;
      i0 += i1;
      i1 = l3;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    i0 = l9;
    if (i0) {
      i0 = l4;
      i1 = 28u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      i1 = 2u;
      i0 <<= (i1 & 31);
      i1 = 5008u;
      i0 += i1;
      l5 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      i1 = l4;
      i0 = i0 == i1;
      if (i0) {
        i0 = l5;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l2;
        i0 = !(i0);
        if (i0) {
          i0 = 4708u;
          i1 = 4708u;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i2 = 1u;
          i3 = l3;
          i2 <<= (i3 & 31);
          i3 = 4294967295u;
          i2 ^= i3;
          i1 &= i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          goto B15;
        }
      } else {
        i0 = l9;
        i1 = 16u;
        i0 += i1;
        l3 = i0;
        i1 = l9;
        i2 = 20u;
        i1 += i2;
        i2 = l3;
        i2 = i32_load(Z_envZ_memory, (u64)(i2));
        i3 = l4;
        i2 = i2 == i3;
        i0 = i2 ? i0 : i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l2;
        i0 = !(i0);
        if (i0) {goto B15;}
      }
      i0 = l2;
      i1 = 24u;
      i0 += i1;
      i1 = l9;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l4;
      i1 = 16u;
      i0 += i1;
      l5 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      if (i0) {
        i0 = l2;
        i1 = 16u;
        i0 += i1;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        i1 = 24u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
      i0 = l5;
      i1 = 4u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      if (i0) {
        i0 = l2;
        i1 = 20u;
        i0 += i1;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        i1 = 24u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
    }
    B15:;
  }
  i0 = l8;
  i1 = p1;
  i0 -= i1;
  l2 = i0;
  i1 = 16u;
  i0 = i0 < i1;
  if (i0) {
    i0 = l7;
    i1 = l6;
    i2 = 1u;
    i1 &= i2;
    i2 = l8;
    i1 |= i2;
    i2 = 2u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l8;
    i0 += i1;
    i1 = 4u;
    i0 += i1;
    p1 = i0;
    i1 = p1;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
  } else {
    i0 = l7;
    i1 = p1;
    i2 = l6;
    i3 = 1u;
    i2 &= i3;
    i1 |= i2;
    i2 = 2u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = p1;
    i0 += i1;
    p1 = i0;
    i1 = 4u;
    i0 += i1;
    i1 = l2;
    i2 = 3u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l8;
    i0 += i1;
    i1 = 4u;
    i0 += i1;
    l3 = i0;
    i1 = l3;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    i1 = l2;
    f21(i0, i1);
    i0 = p0;
  }
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static void f21(u32 p0, u32 p1) {
  u32 l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  i0 = p0;
  i1 = p1;
  i0 += i1;
  l5 = i0;
  i0 = p0;
  i1 = 4u;
  i0 += i1;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l3 = i0;
  i1 = 1u;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {
    i0 = p0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l2 = i0;
    i0 = l3;
    i1 = 3u;
    i0 &= i1;
    i0 = !(i0);
    if (i0) {
      goto Bfunc;
    }
    i0 = p1;
    i1 = l2;
    i0 += i1;
    p1 = i0;
    i0 = p0;
    i1 = l2;
    i0 -= i1;
    p0 = i0;
    i1 = 4724u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 == i1;
    if (i0) {
      i0 = l5;
      i1 = 4u;
      i0 += i1;
      l2 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      i1 = 3u;
      i0 &= i1;
      i1 = 3u;
      i0 = i0 != i1;
      if (i0) {goto B1;}
      i0 = 4712u;
      i1 = p1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l2;
      i1 = l3;
      i2 = 4294967294u;
      i1 &= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = 4u;
      i0 += i1;
      i1 = p1;
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l5;
      i1 = p1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      goto Bfunc;
    }
    i0 = l2;
    i1 = 3u;
    i0 >>= (i1 & 31);
    l4 = i0;
    i0 = l2;
    i1 = 256u;
    i0 = i0 < i1;
    if (i0) {
      i0 = p0;
      i1 = 8u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l2 = i0;
      i1 = p0;
      i2 = 12u;
      i1 += i2;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      l3 = i1;
      i0 = i0 == i1;
      if (i0) {
        i0 = 4704u;
        i1 = 4704u;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i2 = 1u;
        i3 = l4;
        i2 <<= (i3 & 31);
        i3 = 4294967295u;
        i2 ^= i3;
        i1 &= i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        goto B1;
      } else {
        i0 = l2;
        i1 = 12u;
        i0 += i1;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        i1 = 8u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        goto B1;
      }
      UNREACHABLE;
    }
    i0 = p0;
    i1 = 24u;
    i0 += i1;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l7 = i0;
    i0 = p0;
    i1 = p0;
    i2 = 12u;
    i1 += i2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    l2 = i1;
    i0 = i0 == i1;
    if (i0) {
      i0 = p0;
      i1 = 16u;
      i0 += i1;
      l3 = i0;
      i1 = 4u;
      i0 += i1;
      l4 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l2 = i0;
      if (i0) {
        i0 = l4;
        l3 = i0;
      } else {
        i0 = l3;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l2 = i0;
        i0 = !(i0);
        if (i0) {
          i0 = 0u;
          l2 = i0;
          goto B7;
        }
      }
      L10: 
        i0 = l2;
        i1 = 20u;
        i0 += i1;
        l4 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l6 = i0;
        if (i0) {
          i0 = l4;
          l3 = i0;
          i0 = l6;
        } else {
          i0 = l2;
          i1 = 16u;
          i0 += i1;
          l4 = i0;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l6 = i0;
          i0 = !(i0);
          if (i0) {goto B11;}
          i0 = l4;
          l3 = i0;
          i0 = l6;
        }
        l2 = i0;
        goto L10;
        B11:;
      i0 = l3;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      B7:;
    } else {
      i0 = p0;
      i1 = 8u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      i1 = 12u;
      i0 += i1;
      i1 = l2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l2;
      i1 = 8u;
      i0 += i1;
      i1 = l3;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    i0 = l7;
    if (i0) {
      i0 = p0;
      i1 = p0;
      i2 = 28u;
      i1 += i2;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      l3 = i1;
      i2 = 2u;
      i1 <<= (i2 & 31);
      i2 = 5008u;
      i1 += i2;
      l4 = i1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i0 = i0 == i1;
      if (i0) {
        i0 = l4;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l2;
        i0 = !(i0);
        if (i0) {
          i0 = 4708u;
          i1 = 4708u;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i2 = 1u;
          i3 = l3;
          i2 <<= (i3 & 31);
          i3 = 4294967295u;
          i2 ^= i3;
          i1 &= i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          goto B1;
        }
      } else {
        i0 = l7;
        i1 = 16u;
        i0 += i1;
        l3 = i0;
        i1 = l7;
        i2 = 20u;
        i1 += i2;
        i2 = p0;
        i3 = l3;
        i3 = i32_load(Z_envZ_memory, (u64)(i3));
        i2 = i2 == i3;
        i0 = i2 ? i0 : i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l2;
        i0 = !(i0);
        if (i0) {goto B1;}
      }
      i0 = l2;
      i1 = 24u;
      i0 += i1;
      i1 = l7;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = 16u;
      i0 += i1;
      l4 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      if (i0) {
        i0 = l2;
        i1 = 16u;
        i0 += i1;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        i1 = 24u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
      i0 = l4;
      i1 = 4u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      if (i0) {
        i0 = l2;
        i1 = 20u;
        i0 += i1;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        i1 = 24u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
    }
    B1:;
  }
  i0 = l5;
  i1 = 4u;
  i0 += i1;
  l2 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l7 = i0;
  i1 = 2u;
  i0 &= i1;
  if (i0) {
    i0 = l2;
    i1 = l7;
    i2 = 4294967294u;
    i1 &= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 4u;
    i0 += i1;
    i1 = p1;
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = p1;
    i0 += i1;
    i1 = p1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    l3 = i0;
  } else {
    i0 = l5;
    i1 = 4728u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 == i1;
    if (i0) {
      i0 = 4716u;
      i1 = p1;
      i2 = 4716u;
      i2 = i32_load(Z_envZ_memory, (u64)(i2));
      i1 += i2;
      p1 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4728u;
      i1 = p0;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = 4u;
      i0 += i1;
      i1 = p1;
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4724u;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      i1 = p0;
      i0 = i0 != i1;
      if (i0) {
        goto Bfunc;
      }
      i0 = 4724u;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4712u;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      goto Bfunc;
    }
    i0 = l5;
    i1 = 4724u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 == i1;
    if (i0) {
      i0 = 4712u;
      i1 = p1;
      i2 = 4712u;
      i2 = i32_load(Z_envZ_memory, (u64)(i2));
      i1 += i2;
      p1 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4724u;
      i1 = p0;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = 4u;
      i0 += i1;
      i1 = p1;
      i2 = 1u;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = p1;
      i0 += i1;
      i1 = p1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      goto Bfunc;
    }
    i0 = l7;
    i1 = 3u;
    i0 >>= (i1 & 31);
    l4 = i0;
    i0 = l7;
    i1 = 256u;
    i0 = i0 < i1;
    if (i0) {
      i0 = l5;
      i1 = 8u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l2 = i0;
      i1 = l5;
      i2 = 12u;
      i1 += i2;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      l3 = i1;
      i0 = i0 == i1;
      if (i0) {
        i0 = 4704u;
        i1 = 4704u;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i2 = 1u;
        i3 = l4;
        i2 <<= (i3 & 31);
        i3 = 4294967295u;
        i2 ^= i3;
        i1 &= i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      } else {
        i0 = l2;
        i1 = 12u;
        i0 += i1;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        i1 = 8u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
    } else {
      i0 = l5;
      i1 = 24u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l8 = i0;
      i0 = l5;
      i1 = 12u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l2 = i0;
      i1 = l5;
      i0 = i0 == i1;
      if (i0) {
        i0 = l5;
        i1 = 16u;
        i0 += i1;
        l3 = i0;
        i1 = 4u;
        i0 += i1;
        l4 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l2 = i0;
        if (i0) {
          i0 = l4;
          l3 = i0;
        } else {
          i0 = l3;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l2 = i0;
          i0 = !(i0);
          if (i0) {
            i0 = 0u;
            l2 = i0;
            goto B26;
          }
        }
        L29: 
          i0 = l2;
          i1 = 20u;
          i0 += i1;
          l4 = i0;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l6 = i0;
          if (i0) {
            i0 = l4;
            l3 = i0;
            i0 = l6;
          } else {
            i0 = l2;
            i1 = 16u;
            i0 += i1;
            l4 = i0;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
            l6 = i0;
            i0 = !(i0);
            if (i0) {goto B30;}
            i0 = l4;
            l3 = i0;
            i0 = l6;
          }
          l2 = i0;
          goto L29;
          B30:;
        i0 = l3;
        i1 = 0u;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        B26:;
      } else {
        i0 = l5;
        i1 = 8u;
        i0 += i1;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l3 = i0;
        i1 = 12u;
        i0 += i1;
        i1 = l2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l2;
        i1 = 8u;
        i0 += i1;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
      i0 = l8;
      if (i0) {
        i0 = l5;
        i1 = 28u;
        i0 += i1;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l3 = i0;
        i1 = 2u;
        i0 <<= (i1 & 31);
        i1 = 5008u;
        i0 += i1;
        l4 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        i1 = l5;
        i0 = i0 == i1;
        if (i0) {
          i0 = l4;
          i1 = l2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l2;
          i0 = !(i0);
          if (i0) {
            i0 = 4708u;
            i1 = 4708u;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            i2 = 1u;
            i3 = l3;
            i2 <<= (i3 & 31);
            i3 = 4294967295u;
            i2 ^= i3;
            i1 &= i2;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            goto B24;
          }
        } else {
          i0 = l8;
          i1 = 16u;
          i0 += i1;
          l3 = i0;
          i1 = l8;
          i2 = 20u;
          i1 += i2;
          i2 = l3;
          i2 = i32_load(Z_envZ_memory, (u64)(i2));
          i3 = l5;
          i2 = i2 == i3;
          i0 = i2 ? i0 : i1;
          i1 = l2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l2;
          i0 = !(i0);
          if (i0) {goto B24;}
        }
        i0 = l2;
        i1 = 24u;
        i0 += i1;
        i1 = l8;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l5;
        i1 = 16u;
        i0 += i1;
        l4 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l3 = i0;
        if (i0) {
          i0 = l2;
          i1 = 16u;
          i0 += i1;
          i1 = l3;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l3;
          i1 = 24u;
          i0 += i1;
          i1 = l2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
        }
        i0 = l4;
        i1 = 4u;
        i0 += i1;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l3 = i0;
        if (i0) {
          i0 = l2;
          i1 = 20u;
          i0 += i1;
          i1 = l3;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l3;
          i1 = 24u;
          i0 += i1;
          i1 = l2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
        }
      }
      B24:;
    }
    i0 = p0;
    i1 = 4u;
    i0 += i1;
    i1 = p1;
    i2 = l7;
    i3 = 4294967288u;
    i2 &= i3;
    i1 += i2;
    l3 = i1;
    i2 = 1u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l3;
    i0 += i1;
    i1 = l3;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 4724u;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 == i1;
    if (i0) {
      i0 = 4712u;
      i1 = l3;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      goto Bfunc;
    }
  }
  i0 = l3;
  i1 = 3u;
  i0 >>= (i1 & 31);
  l2 = i0;
  i0 = l3;
  i1 = 256u;
  i0 = i0 < i1;
  if (i0) {
    i0 = l2;
    i1 = 3u;
    i0 <<= (i1 & 31);
    i1 = 4744u;
    i0 += i1;
    p1 = i0;
    i0 = 4704u;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i1 = 1u;
    i2 = l2;
    i1 <<= (i2 & 31);
    l2 = i1;
    i0 &= i1;
    if (i0) {
      i0 = p1;
      i1 = 8u;
      i0 += i1;
      l3 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
    } else {
      i0 = 4704u;
      i1 = l2;
      i2 = l3;
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p1;
      i1 = 8u;
      i0 += i1;
      l3 = i0;
      i0 = p1;
    }
    l2 = i0;
    i0 = l3;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l2;
    i1 = 12u;
    i0 += i1;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 8u;
    i0 += i1;
    i1 = l2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 12u;
    i0 += i1;
    i1 = p1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    goto Bfunc;
  }
  i0 = l3;
  i1 = 8u;
  i0 >>= (i1 & 31);
  p1 = i0;
  if (i0) {
    i0 = l3;
    i1 = 16777215u;
    i0 = i0 > i1;
    if (i0) {
      i0 = 31u;
    } else {
      i0 = p1;
      i1 = p1;
      i2 = 1048320u;
      i1 += i2;
      i2 = 16u;
      i1 >>= (i2 & 31);
      i2 = 8u;
      i1 &= i2;
      l2 = i1;
      i0 <<= (i1 & 31);
      l4 = i0;
      i1 = 520192u;
      i0 += i1;
      i1 = 16u;
      i0 >>= (i1 & 31);
      i1 = 4u;
      i0 &= i1;
      p1 = i0;
      i0 = 14u;
      i1 = p1;
      i2 = l2;
      i1 |= i2;
      i2 = l4;
      i3 = p1;
      i2 <<= (i3 & 31);
      p1 = i2;
      i3 = 245760u;
      i2 += i3;
      i3 = 16u;
      i2 >>= (i3 & 31);
      i3 = 2u;
      i2 &= i3;
      l2 = i2;
      i1 |= i2;
      i0 -= i1;
      i1 = p1;
      i2 = l2;
      i1 <<= (i2 & 31);
      i2 = 15u;
      i1 >>= (i2 & 31);
      i0 += i1;
      p1 = i0;
      i1 = 1u;
      i0 <<= (i1 & 31);
      i1 = l3;
      i2 = p1;
      i3 = 7u;
      i2 += i3;
      i1 >>= (i2 & 31);
      i2 = 1u;
      i1 &= i2;
      i0 |= i1;
    }
  } else {
    i0 = 0u;
  }
  l2 = i0;
  i1 = 2u;
  i0 <<= (i1 & 31);
  i1 = 5008u;
  i0 += i1;
  p1 = i0;
  i0 = p0;
  i1 = 28u;
  i0 += i1;
  i1 = l2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 20u;
  i0 += i1;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 16u;
  i0 += i1;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 4708u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l4 = i0;
  i1 = 1u;
  i2 = l2;
  i1 <<= (i2 & 31);
  l6 = i1;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {
    i0 = 4708u;
    i1 = l4;
    i2 = l6;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 24u;
    i0 += i1;
    i1 = p1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 12u;
    i0 += i1;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 8u;
    i0 += i1;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    goto Bfunc;
  }
  i0 = l3;
  i1 = p1;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  p1 = i1;
  i2 = 4u;
  i1 += i2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i2 = 4294967288u;
  i1 &= i2;
  i0 = i0 == i1;
  if (i0) {
    i0 = p1;
    l2 = i0;
  } else {
    i0 = l3;
    i1 = 0u;
    i2 = 25u;
    i3 = l2;
    i4 = 1u;
    i3 >>= (i4 & 31);
    i2 -= i3;
    i3 = l2;
    i4 = 31u;
    i3 = i3 == i4;
    i1 = i3 ? i1 : i2;
    i0 <<= (i1 & 31);
    l4 = i0;
    L45: 
      i0 = p1;
      i1 = 16u;
      i0 += i1;
      i1 = l4;
      i2 = 31u;
      i1 >>= (i2 & 31);
      i2 = 2u;
      i1 <<= (i2 & 31);
      i0 += i1;
      l6 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l2 = i0;
      if (i0) {
        i0 = l4;
        i1 = 1u;
        i0 <<= (i1 & 31);
        l4 = i0;
        i0 = l3;
        i1 = l2;
        i2 = 4u;
        i1 += i2;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i2 = 4294967288u;
        i1 &= i2;
        i0 = i0 == i1;
        if (i0) {goto B44;}
        i0 = l2;
        p1 = i0;
        goto L45;
      }
    i0 = l6;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 24u;
    i0 += i1;
    i1 = p1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 12u;
    i0 += i1;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 8u;
    i0 += i1;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    goto Bfunc;
    B44:;
  }
  i0 = l2;
  i1 = 8u;
  i0 += i1;
  p1 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l3 = i0;
  i1 = 12u;
  i0 += i1;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p1;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 8u;
  i0 += i1;
  i1 = l3;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 12u;
  i0 += i1;
  i1 = l2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 24u;
  i0 += i1;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  Bfunc:;
  FUNC_EPILOGUE;
}

static u32 f22(u32 p0) {
  u32 l1 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1;
  i0 = g7;
  l1 = i0;
  i0 = g7;
  i1 = 16u;
  i0 += i1;
  g7 = i0;
  i0 = l1;
  i1 = p0;
  i1 = i32_load(Z_envZ_memory, (u64)(i1 + 60));
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 6u;
  i1 = l1;
  i0 = (*Z_envZ____syscall6Z_iii)(i0, i1);
  i0 = f25(i0);
  p0 = i0;
  i0 = l1;
  g7 = i0;
  i0 = p0;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f23(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0, l9 = 0, l10 = 0, 
      l11 = 0, l12 = 0, l13 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  i0 = g7;
  l7 = i0;
  i0 = g7;
  i1 = 48u;
  i0 += i1;
  g7 = i0;
  i0 = l7;
  i1 = 32u;
  i0 += i1;
  l5 = i0;
  i0 = l7;
  l3 = i0;
  i1 = p0;
  i2 = 28u;
  i1 += i2;
  l10 = i1;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  l4 = i1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 4u;
  i0 += i1;
  i1 = p0;
  i2 = 20u;
  i1 += i2;
  l11 = i1;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i2 = l4;
  i1 -= i2;
  l4 = i1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 8u;
  i0 += i1;
  i1 = p1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 12u;
  i0 += i1;
  i1 = p2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 16u;
  i0 += i1;
  p1 = i0;
  i1 = p0;
  i2 = 60u;
  i1 += i2;
  l12 = i1;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p1;
  i1 = 4u;
  i0 += i1;
  i1 = l3;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p1;
  i1 = 8u;
  i0 += i1;
  i1 = 2u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p2;
  i1 = l4;
  i0 += i1;
  l4 = i0;
  i1 = 146u;
  i2 = p1;
  i1 = (*Z_envZ____syscall146Z_iii)(i1, i2);
  i1 = f25(i1);
  l6 = i1;
  i0 = i0 == i1;
  if (i0) {goto B1;}
  i0 = 2u;
  l8 = i0;
  i0 = l3;
  p1 = i0;
  i0 = l6;
  l3 = i0;
  L2: 
    i0 = l3;
    i1 = 0u;
    i0 = (u32)((s32)i0 >= (s32)i1);
    if (i0) {
      i0 = p1;
      i1 = 8u;
      i0 += i1;
      i1 = p1;
      i2 = l3;
      i3 = p1;
      i4 = 4u;
      i3 += i4;
      i3 = i32_load(Z_envZ_memory, (u64)(i3));
      l9 = i3;
      i2 = i2 > i3;
      l6 = i2;
      i0 = i2 ? i0 : i1;
      p1 = i0;
      i1 = l3;
      i2 = l9;
      i3 = 0u;
      i4 = l6;
      i2 = i4 ? i2 : i3;
      i1 -= i2;
      l9 = i1;
      i2 = p1;
      i2 = i32_load(Z_envZ_memory, (u64)(i2));
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p1;
      i1 = 4u;
      i0 += i1;
      l13 = i0;
      i1 = l13;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = l9;
      i1 -= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l5;
      i1 = l12;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l5;
      i1 = 4u;
      i0 += i1;
      i1 = p1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l5;
      i1 = 8u;
      i0 += i1;
      i1 = l8;
      i2 = l6;
      i3 = 31u;
      i2 <<= (i3 & 31);
      i3 = 31u;
      i2 = (u32)((s32)i2 >> (i3 & 31));
      i1 += i2;
      l8 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l4;
      i1 = l3;
      i0 -= i1;
      l4 = i0;
      i1 = 146u;
      i2 = l5;
      i1 = (*Z_envZ____syscall146Z_iii)(i1, i2);
      i1 = f25(i1);
      l3 = i1;
      i0 = i0 == i1;
      if (i0) {goto B1;}
      goto L2;
    }
  i0 = p0;
  i1 = 16u;
  i0 += i1;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l10;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l11;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p0;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i2 = 32u;
  i1 |= i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l8;
  i1 = 2u;
  i0 = i0 == i1;
  if (i0) {
    i0 = 0u;
  } else {
    i0 = p2;
    i1 = p1;
    i2 = 4u;
    i1 += i2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 -= i1;
  }
  p2 = i0;
  goto B0;
  B1:;
  i0 = p0;
  i1 = 16u;
  i0 += i1;
  i1 = p0;
  i2 = 44u;
  i1 += i2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  p1 = i1;
  i2 = p0;
  i3 = 48u;
  i2 += i3;
  i2 = i32_load(Z_envZ_memory, (u64)(i2));
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l10;
  i1 = p1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l11;
  i1 = p1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  B0:;
  i0 = l7;
  g7 = i0;
  i0 = p2;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f24(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = g7;
  l4 = i0;
  i0 = g7;
  i1 = 32u;
  i0 += i1;
  g7 = i0;
  i0 = l4;
  l3 = i0;
  i1 = p0;
  i2 = 60u;
  i1 += i2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 4u;
  i0 += i1;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 8u;
  i0 += i1;
  i1 = p1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 12u;
  i0 += i1;
  i1 = l3;
  i2 = 20u;
  i1 += i2;
  p0 = i1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 16u;
  i0 += i1;
  i1 = p2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 140u;
  i1 = l3;
  i0 = (*Z_envZ____syscall140Z_iii)(i0, i1);
  i0 = f25(i0);
  i1 = 0u;
  i0 = (u32)((s32)i0 < (s32)i1);
  if (i0) {
    i0 = p0;
    i1 = 4294967295u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4294967295u;
  } else {
    i0 = p0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
  }
  p0 = i0;
  i0 = l4;
  g7 = i0;
  i0 = p0;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f25(u32 p0) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  i1 = 4294963200u;
  i0 = i0 > i1;
  if (i0) {
    i0 = 5200u;
    i1 = 0u;
    i2 = p0;
    i1 -= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4294967295u;
  } else {
    i0 = p0;
  }
  FUNC_EPILOGUE;
  return i0;
}

static u32 ___errno_location(void) {
  FUNC_PROLOGUE;
  u32 i0;
  i0 = 5200u;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f27(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0, l5 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = g7;
  l4 = i0;
  i0 = g7;
  i1 = 32u;
  i0 += i1;
  g7 = i0;
  i0 = l4;
  l3 = i0;
  i1 = 16u;
  i0 += i1;
  l5 = i0;
  i0 = p0;
  i1 = 36u;
  i0 += i1;
  i1 = 4u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  i1 = 64u;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {
    i0 = l3;
    i1 = p0;
    i2 = 60u;
    i1 += i2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 4u;
    i0 += i1;
    i1 = 21523u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    i1 = 8u;
    i0 += i1;
    i1 = l5;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 54u;
    i1 = l3;
    i0 = (*Z_envZ____syscall54Z_iii)(i0, i1);
    if (i0) {
      i0 = p0;
      i1 = 75u;
      i0 += i1;
      i1 = 4294967295u;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
    }
  }
  i0 = p0;
  i1 = p1;
  i2 = p2;
  i0 = f23(i0, i1, i2);
  p0 = i0;
  i0 = l4;
  g7 = i0;
  i0 = p0;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f28(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  i0 = g7;
  l7 = i0;
  i0 = g7;
  i1 = 32u;
  i0 += i1;
  g7 = i0;
  i0 = l7;
  l3 = i0;
  i1 = p1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 4u;
  i0 += i1;
  l6 = i0;
  i1 = p2;
  i2 = p0;
  i3 = 48u;
  i2 += i3;
  l8 = i2;
  i2 = i32_load(Z_envZ_memory, (u64)(i2));
  l4 = i2;
  i3 = 0u;
  i2 = i2 != i3;
  i1 -= i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 8u;
  i0 += i1;
  i1 = p0;
  i2 = 44u;
  i1 += i2;
  l5 = i1;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 12u;
  i0 += i1;
  i1 = l4;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = 16u;
  i0 += i1;
  l4 = i0;
  i1 = p0;
  i2 = 60u;
  i1 += i2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l4;
  i1 = 4u;
  i0 += i1;
  i1 = l3;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l4;
  i1 = 8u;
  i0 += i1;
  i1 = 2u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 145u;
  i1 = l4;
  i0 = (*Z_envZ____syscall145Z_iii)(i0, i1);
  i0 = f25(i0);
  l3 = i0;
  i1 = 1u;
  i0 = (u32)((s32)i0 < (s32)i1);
  if (i0) {
    i0 = p0;
    i1 = p0;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = l3;
    i3 = 48u;
    i2 &= i3;
    i3 = 16u;
    i2 ^= i3;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l3;
    p2 = i0;
  } else {
    i0 = l3;
    i1 = l6;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    l6 = i1;
    i0 = i0 > i1;
    if (i0) {
      i0 = p0;
      i1 = 4u;
      i0 += i1;
      l4 = i0;
      i1 = l5;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      l5 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = 8u;
      i0 += i1;
      i1 = l5;
      i2 = l3;
      i3 = l6;
      i2 -= i3;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l8;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      if (i0) {
        i0 = l4;
        i1 = l5;
        i2 = 1u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p1;
        i1 = p2;
        i2 = 4294967295u;
        i1 += i2;
        i0 += i1;
        i1 = l5;
        i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
        i32_store8(Z_envZ_memory, (u64)(i0), i1);
      }
    } else {
      i0 = l3;
      p2 = i0;
    }
  }
  i0 = l7;
  g7 = i0;
  i0 = p2;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f29(u32 p0) {
  FUNC_PROLOGUE;
  u32 i0, i1;
  i0 = p0;
  i1 = 4294967248u;
  i0 += i1;
  i1 = 10u;
  i0 = i0 < i1;
  FUNC_EPILOGUE;
  return i0;
}

static void f30(u32 p0, u32 p1) {
  u32 l2 = 0, l3 = 0, l4 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  i0 = p0;
  i1 = 104u;
  i0 += i1;
  i1 = p1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 108u;
  i0 += i1;
  i1 = p0;
  i2 = 8u;
  i1 += i2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  l2 = i1;
  i2 = p0;
  i3 = 4u;
  i2 += i3;
  i2 = i32_load(Z_envZ_memory, (u64)(i2));
  l3 = i2;
  i1 -= i2;
  l4 = i1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p1;
  i1 = 0u;
  i0 = i0 != i1;
  i1 = l4;
  i2 = p1;
  i1 = (u32)((s32)i1 > (s32)i2);
  i0 &= i1;
  if (i0) {
    i0 = p0;
    i1 = 100u;
    i0 += i1;
    i1 = p1;
    i2 = l3;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  } else {
    i0 = p0;
    i1 = 100u;
    i0 += i1;
    i1 = l2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  }
  FUNC_EPILOGUE;
}

static u64 f31(u32 p0, u32 p1) {
  u32 l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0, l9 = 0;
  u64 l10 = 0, l11 = 0, l12 = 0, l13 = 0, l14 = 0, l15 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  u64 j0, j1, j2, j3;
  j0 = 18446744073709551615ull;
  l11 = j0;
  i0 = p1;
  i1 = 36u;
  i0 = i0 > i1;
  if (i0) {
    i0 = 5200u;
    i1 = 22u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    j0 = 0ull;
    l11 = j0;
  } else {
    i0 = p0;
    i1 = 4u;
    i0 += i1;
    l4 = i0;
    i0 = p0;
    i1 = 100u;
    i0 += i1;
    l6 = i0;
    L2: 
      i0 = l4;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l7 = i0;
      i1 = l6;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i0 = i0 < i1;
      if (i0) {
        i0 = l4;
        i1 = l7;
        i2 = 1u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l7;
        i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
      } else {
        i0 = p0;
        i0 = f32_0(i0);
      }
      l3 = i0;
      i0 = f33(i0);
      if (i0) {goto L2;}
    i0 = l3;
    i1 = 43u;
    i0 -= i1;
    switch (i0) {
      case 0: goto B6;
      case 1: goto B5;
      case 2: goto B6;
      default: goto B5;
    }
    B6:;
    i0 = l3;
    i1 = 45u;
    i0 = i0 == i1;
    i1 = 31u;
    i0 <<= (i1 & 31);
    i1 = 31u;
    i0 = (u32)((s32)i0 >> (i1 & 31));
    l7 = i0;
    i0 = l4;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i1 = l6;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 < i1;
    if (i0) {
      i0 = l4;
      i1 = l3;
      i2 = 1u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      goto B4;
    } else {
      i0 = p0;
      i0 = f32_0(i0);
      l3 = i0;
      goto B4;
    }
    UNREACHABLE;
    B5:;
    i0 = 0u;
    l7 = i0;
    B4:;
    i0 = p1;
    i0 = !(i0);
    l5 = i0;
    i0 = p1;
    i1 = 16u;
    i0 |= i1;
    i1 = 16u;
    i0 = i0 == i1;
    i1 = l3;
    i2 = 48u;
    i1 = i1 == i2;
    i0 &= i1;
    if (i0) {
      i0 = l4;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      i1 = l6;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i0 = i0 < i1;
      if (i0) {
        i0 = l4;
        i1 = l3;
        i2 = 1u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
      } else {
        i0 = p0;
        i0 = f32_0(i0);
      }
      l3 = i0;
      i1 = 32u;
      i0 |= i1;
      i1 = 120u;
      i0 = i0 != i1;
      if (i0) {
        i0 = l5;
        if (i0) {
          i0 = l3;
          l2 = i0;
          i0 = 8u;
          p1 = i0;
          goto B10;
        } else {
          i0 = l3;
          l2 = i0;
          goto B12;
        }
        UNREACHABLE;
      }
      i0 = l4;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      p1 = i0;
      i1 = l6;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i0 = i0 < i1;
      if (i0) {
        i0 = l4;
        i1 = p1;
        i2 = 1u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p1;
        i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
      } else {
        i0 = p0;
        i0 = f32_0(i0);
      }
      p1 = i0;
      i1 = 1233u;
      i0 += i1;
      i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
      i1 = 15u;
      i0 = (u32)((s32)i0 > (s32)i1);
      if (i0) {
        i0 = l6;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        if (i0) {
          i0 = l4;
          i1 = l4;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i2 = 4294967295u;
          i1 += i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
        }
        i0 = p0;
        i1 = 0u;
        f30(i0, i1);
        j0 = 0ull;
        l11 = j0;
        goto B1;
      } else {
        i0 = p1;
        l2 = i0;
        i0 = 16u;
        p1 = i0;
        goto B10;
      }
      UNREACHABLE;
      B12:;
    } else {
      i0 = 10u;
      i1 = p1;
      i2 = l5;
      i0 = i2 ? i0 : i1;
      p1 = i0;
      i1 = l3;
      i2 = 1233u;
      i1 += i2;
      i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
      i0 = i0 > i1;
      if (i0) {
        i0 = l3;
      } else {
        i0 = l6;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        if (i0) {
          i0 = l4;
          i1 = l4;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i2 = 4294967295u;
          i1 += i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
        }
        i0 = p0;
        i1 = 0u;
        f30(i0, i1);
        i0 = 5200u;
        i1 = 22u;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        j0 = 0ull;
        l11 = j0;
        goto B1;
      }
      l2 = i0;
    }
    i0 = p1;
    i1 = 10u;
    i0 = i0 != i1;
    if (i0) {goto B10;}
    i0 = l2;
    i1 = 4294967248u;
    i0 += i1;
    l2 = i0;
    i1 = 10u;
    i0 = i0 < i1;
    if (i0) {
      i0 = 0u;
      p1 = i0;
      L22: 
        i0 = p1;
        i1 = 10u;
        i0 *= i1;
        i1 = l2;
        i0 += i1;
        p1 = i0;
        i0 = l4;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l2 = i0;
        i1 = l6;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i0 = i0 < i1;
        if (i0) {
          i0 = l4;
          i1 = l2;
          i2 = 1u;
          i1 += i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l2;
          i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
        } else {
          i0 = p0;
          i0 = f32_0(i0);
        }
        l3 = i0;
        i1 = 4294967248u;
        i0 += i1;
        l2 = i0;
        i1 = 10u;
        i0 = i0 < i1;
        i1 = p1;
        i2 = 429496729u;
        i1 = i1 < i2;
        i0 &= i1;
        if (i0) {goto L22;}
      i0 = p1;
      j0 = (u64)(i0);
      l10 = j0;
      i0 = l2;
      i1 = 10u;
      i0 = i0 < i1;
      if (i0) {
        i0 = l3;
        p1 = i0;
        L25: 
          j0 = l10;
          j1 = 10ull;
          j0 *= j1;
          l12 = j0;
          i1 = l2;
          j1 = (u64)(s64)(s32)(i1);
          l13 = j1;
          j2 = 18446744073709551615ull;
          j1 ^= j2;
          i0 = j0 > j1;
          if (i0) {
            i0 = 10u;
            l2 = i0;
            goto B9;
          }
          j0 = l12;
          j1 = l13;
          j0 += j1;
          l10 = j0;
          i0 = l4;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          p1 = i0;
          i1 = l6;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i0 = i0 < i1;
          if (i0) {
            i0 = l4;
            i1 = p1;
            i2 = 1u;
            i1 += i2;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            i0 = p1;
            i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
          } else {
            i0 = p0;
            i0 = f32_0(i0);
          }
          p1 = i0;
          i1 = 4294967248u;
          i0 += i1;
          l2 = i0;
          i1 = 10u;
          i0 = i0 < i1;
          j1 = l10;
          j2 = 1844674407370955162ull;
          i1 = j1 < j2;
          i0 &= i1;
          if (i0) {goto L25;}
        i0 = l2;
        i1 = 9u;
        i0 = i0 <= i1;
        if (i0) {
          i0 = 10u;
          l2 = i0;
          goto B9;
        }
      }
    }
    goto B8;
    B10:;
    i0 = p1;
    i1 = p1;
    i2 = 4294967295u;
    i1 += i2;
    i0 &= i1;
    i0 = !(i0);
    if (i0) {
      i0 = p1;
      i1 = 23u;
      i0 *= i1;
      i1 = 5u;
      i0 >>= (i1 & 31);
      i1 = 7u;
      i0 &= i1;
      i1 = 2544u;
      i0 += i1;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      l9 = i0;
      i0 = p1;
      i1 = l2;
      i2 = 1233u;
      i1 += i2;
      i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
      l8 = i1;
      i2 = 255u;
      i1 &= i2;
      l5 = i1;
      i0 = i0 > i1;
      if (i0) {
        i0 = 0u;
        l3 = i0;
        i0 = l5;
        l2 = i0;
        L31: 
          i0 = l3;
          i1 = l9;
          i0 <<= (i1 & 31);
          i1 = l2;
          i0 |= i1;
          l3 = i0;
          i1 = 134217728u;
          i0 = i0 < i1;
          i1 = p1;
          i2 = l4;
          i2 = i32_load(Z_envZ_memory, (u64)(i2));
          l2 = i2;
          i3 = l6;
          i3 = i32_load(Z_envZ_memory, (u64)(i3));
          i2 = i2 < i3;
          if (i2) {
            i2 = l4;
            i3 = l2;
            i4 = 1u;
            i3 += i4;
            i32_store(Z_envZ_memory, (u64)(i2), i3);
            i2 = l2;
            i2 = i32_load8_u(Z_envZ_memory, (u64)(i2));
          } else {
            i2 = p0;
            i2 = f32_0(i2);
          }
          l5 = i2;
          i3 = 1233u;
          i2 += i3;
          i2 = i32_load8_s(Z_envZ_memory, (u64)(i2));
          l8 = i2;
          i3 = 255u;
          i2 &= i3;
          l2 = i2;
          i1 = i1 > i2;
          i0 &= i1;
          if (i0) {goto L31;}
        i0 = l3;
        j0 = (u64)(i0);
        l10 = j0;
        i0 = l5;
        l3 = i0;
        i0 = l2;
        l5 = i0;
        i0 = l8;
      } else {
        i0 = l2;
        l3 = i0;
        i0 = l8;
      }
      l2 = i0;
      i0 = p1;
      i1 = l5;
      i0 = i0 <= i1;
      j1 = 18446744073709551615ull;
      i2 = l9;
      j2 = (u64)(i2);
      l12 = j2;
      j1 >>= (j2 & 63);
      l13 = j1;
      j2 = l10;
      i1 = j1 < j2;
      i0 |= i1;
      if (i0) {
        i0 = p1;
        l2 = i0;
        i0 = l3;
        p1 = i0;
        goto B9;
      }
      L34: 
        i0 = p1;
        i1 = l4;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        l3 = i1;
        i2 = l6;
        i2 = i32_load(Z_envZ_memory, (u64)(i2));
        i1 = i1 < i2;
        if (i1) {
          i1 = l4;
          i2 = l3;
          i3 = 1u;
          i2 += i3;
          i32_store(Z_envZ_memory, (u64)(i1), i2);
          i1 = l3;
          i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
        } else {
          i1 = p0;
          i1 = f32_0(i1);
        }
        l5 = i1;
        i2 = 1233u;
        i1 += i2;
        i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
        l3 = i1;
        i2 = 255u;
        i1 &= i2;
        i0 = i0 <= i1;
        i1 = l2;
        i2 = 255u;
        i1 &= i2;
        j1 = (u64)(i1);
        j2 = l10;
        j3 = l12;
        j2 <<= (j3 & 63);
        j1 |= j2;
        l10 = j1;
        j2 = l13;
        i1 = j1 > j2;
        i0 |= i1;
        if (i0) {
          i0 = p1;
          l2 = i0;
          i0 = l5;
          p1 = i0;
          goto B9;
        } else {
          i0 = l3;
          l2 = i0;
          goto L34;
        }
        UNREACHABLE;
      UNREACHABLE;
    }
    i0 = p1;
    i1 = l2;
    i2 = 1233u;
    i1 += i2;
    i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
    l8 = i1;
    i2 = 255u;
    i1 &= i2;
    l5 = i1;
    i0 = i0 > i1;
    if (i0) {
      i0 = 0u;
      l3 = i0;
      i0 = l5;
      l2 = i0;
      L38: 
        i0 = p1;
        i1 = l3;
        i0 *= i1;
        i1 = l2;
        i0 += i1;
        l3 = i0;
        i1 = 119304647u;
        i0 = i0 < i1;
        i1 = p1;
        i2 = l4;
        i2 = i32_load(Z_envZ_memory, (u64)(i2));
        l2 = i2;
        i3 = l6;
        i3 = i32_load(Z_envZ_memory, (u64)(i3));
        i2 = i2 < i3;
        if (i2) {
          i2 = l4;
          i3 = l2;
          i4 = 1u;
          i3 += i4;
          i32_store(Z_envZ_memory, (u64)(i2), i3);
          i2 = l2;
          i2 = i32_load8_u(Z_envZ_memory, (u64)(i2));
        } else {
          i2 = p0;
          i2 = f32_0(i2);
        }
        l5 = i2;
        i3 = 1233u;
        i2 += i3;
        i2 = i32_load8_s(Z_envZ_memory, (u64)(i2));
        l8 = i2;
        i3 = 255u;
        i2 &= i3;
        l2 = i2;
        i1 = i1 > i2;
        i0 &= i1;
        if (i0) {goto L38;}
      i0 = l3;
      j0 = (u64)(i0);
      l10 = j0;
      i0 = l5;
      l3 = i0;
      i0 = l2;
      l5 = i0;
      i0 = l8;
    } else {
      i0 = l2;
      l3 = i0;
      i0 = l8;
    }
    l2 = i0;
    i0 = p1;
    j0 = (u64)(i0);
    l12 = j0;
    i0 = p1;
    i1 = l5;
    i0 = i0 > i1;
    if (i0) {
      j0 = 18446744073709551615ull;
      j1 = l12;
      j0 = DIV_U(j0, j1);
      l13 = j0;
      L41: 
        j0 = l10;
        j1 = l13;
        i0 = j0 > j1;
        if (i0) {
          i0 = p1;
          l2 = i0;
          i0 = l3;
          p1 = i0;
          goto B9;
        }
        j0 = l10;
        j1 = l12;
        j0 *= j1;
        l14 = j0;
        i1 = l2;
        i2 = 255u;
        i1 &= i2;
        j1 = (u64)(i1);
        l15 = j1;
        j2 = 18446744073709551615ull;
        j1 ^= j2;
        i0 = j0 > j1;
        if (i0) {
          i0 = p1;
          l2 = i0;
          i0 = l3;
          p1 = i0;
          goto B9;
        }
        j0 = l14;
        j1 = l15;
        j0 += j1;
        l10 = j0;
        i0 = p1;
        i1 = l4;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        l2 = i1;
        i2 = l6;
        i2 = i32_load(Z_envZ_memory, (u64)(i2));
        i1 = i1 < i2;
        if (i1) {
          i1 = l4;
          i2 = l2;
          i3 = 1u;
          i2 += i3;
          i32_store(Z_envZ_memory, (u64)(i1), i2);
          i1 = l2;
          i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
        } else {
          i1 = p0;
          i1 = f32_0(i1);
        }
        l3 = i1;
        i2 = 1233u;
        i1 += i2;
        i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
        l2 = i1;
        i2 = 255u;
        i1 &= i2;
        i0 = i0 > i1;
        if (i0) {goto L41;}
        i0 = p1;
        l2 = i0;
        i0 = l3;
    } else {
      i0 = p1;
      l2 = i0;
      i0 = l3;
    }
    p1 = i0;
    B9:;
    i0 = l2;
    i1 = p1;
    i2 = 1233u;
    i1 += i2;
    i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
    i0 = i0 > i1;
    if (i0) {
      L46: 
        i0 = l2;
        i1 = l4;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        p1 = i1;
        i2 = l6;
        i2 = i32_load(Z_envZ_memory, (u64)(i2));
        i1 = i1 < i2;
        if (i1) {
          i1 = l4;
          i2 = p1;
          i3 = 1u;
          i2 += i3;
          i32_store(Z_envZ_memory, (u64)(i1), i2);
          i1 = p1;
          i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
        } else {
          i1 = p0;
          i1 = f32_0(i1);
        }
        i2 = 1233u;
        i1 += i2;
        i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
        i0 = i0 > i1;
        if (i0) {goto L46;}
      i0 = 5200u;
      i1 = 34u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 0u;
      l7 = i0;
      j0 = 18446744073709551615ull;
      l10 = j0;
    }
    B8:;
    i0 = l6;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    if (i0) {
      i0 = l4;
      i1 = l4;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 4294967295u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    j0 = l10;
    j1 = 18446744073709551615ull;
    i0 = j0 >= j1;
    if (i0) {
      i0 = l7;
      i1 = 0u;
      i0 = i0 != i1;
      i1 = 1u;
      i0 |= i1;
      i0 = !(i0);
      if (i0) {
        i0 = 5200u;
        i1 = 34u;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        j0 = 18446744073709551614ull;
        l11 = j0;
        goto B1;
      }
      j0 = l10;
      j1 = 18446744073709551615ull;
      i0 = j0 > j1;
      if (i0) {
        i0 = 5200u;
        i1 = 34u;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        goto B1;
      }
    }
    j0 = l10;
    i1 = l7;
    j1 = (u64)(s64)(s32)(i1);
    l11 = j1;
    j0 ^= j1;
    j1 = l11;
    j0 -= j1;
    l11 = j0;
    B1:;
  }
  j0 = l11;
  FUNC_EPILOGUE;
  return j0;
}

static u32 f32_0(u32 p0) {
  u32 l1 = 0, l2 = 0, l3 = 0, l4 = 0, l5 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  i0 = p0;
  i1 = 104u;
  i0 += i1;
  l3 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l2 = i0;
  if (i0) {
    i0 = p0;
    i1 = 108u;
    i0 += i1;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = l2;
    i0 = (u32)((s32)i0 >= (s32)i1);
    if (i0) {goto B1;}
  }
  i0 = p0;
  i0 = f34(i0);
  l2 = i0;
  i1 = 0u;
  i0 = (u32)((s32)i0 < (s32)i1);
  if (i0) {goto B1;}
  i0 = p0;
  i1 = 8u;
  i0 += i1;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i0 = l3;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l4 = i0;
  if (i0) {
    i0 = l1;
    l3 = i0;
    i0 = l1;
    i1 = p0;
    i2 = 4u;
    i1 += i2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    l5 = i1;
    i0 -= i1;
    i1 = l4;
    i2 = p0;
    i3 = 108u;
    i2 += i3;
    i2 = i32_load(Z_envZ_memory, (u64)(i2));
    i1 -= i2;
    l4 = i1;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {goto B4;}
    i0 = p0;
    i1 = 100u;
    i0 += i1;
    i1 = l5;
    i2 = l4;
    i3 = 4294967295u;
    i2 += i3;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  } else {
    i0 = l1;
    l3 = i0;
    goto B4;
  }
  goto B3;
  B4:;
  i0 = p0;
  i1 = 100u;
  i0 += i1;
  i1 = l1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  B3:;
  i0 = p0;
  i1 = 4u;
  i0 += i1;
  l1 = i0;
  i0 = l3;
  if (i0) {
    i0 = p0;
    i1 = 108u;
    i0 += i1;
    p0 = i0;
    i1 = p0;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = l3;
    i3 = 1u;
    i2 += i3;
    i3 = l1;
    i3 = i32_load(Z_envZ_memory, (u64)(i3));
    p0 = i3;
    i2 -= i3;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  } else {
    i0 = l1;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    p0 = i0;
  }
  i0 = l2;
  i1 = p0;
  i2 = 4294967295u;
  i1 += i2;
  p0 = i1;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
  i0 = i0 != i1;
  if (i0) {
    i0 = p0;
    i1 = l2;
    i32_store8(Z_envZ_memory, (u64)(i0), i1);
  }
  goto B0;
  B1:;
  i0 = p0;
  i1 = 100u;
  i0 += i1;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 4294967295u;
  l2 = i0;
  B0:;
  i0 = l2;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f33(u32 p0) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  i1 = 32u;
  i0 = i0 == i1;
  i1 = p0;
  i2 = 4294967287u;
  i1 += i2;
  i2 = 5u;
  i1 = i1 < i2;
  i0 |= i1;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f34(u32 p0) {
  u32 l1 = 0, l2 = 0, l3 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  i0 = g7;
  l1 = i0;
  i0 = g7;
  i1 = 16u;
  i0 += i1;
  g7 = i0;
  i0 = l1;
  l2 = i0;
  i0 = p0;
  i0 = f35(i0);
  if (i0) {
    i0 = 4294967295u;
  } else {
    i0 = p0;
    i1 = 32u;
    i0 += i1;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i0 = p0;
    i1 = l2;
    i2 = 1u;
    i3 = l3;
    i4 = 7u;
    i3 &= i4;
    i4 = 2u;
    i3 += i4;
    i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, u32, u32), 0, i3, i0, i1, i2);
    i1 = 1u;
    i0 = i0 == i1;
    if (i0) {
      i0 = l2;
      i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    } else {
      i0 = 4294967295u;
    }
  }
  p0 = i0;
  i0 = l1;
  g7 = i0;
  i0 = p0;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f35(u32 p0) {
  u32 l1 = 0, l2 = 0, l3 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  i0 = p0;
  i1 = 74u;
  i0 += i1;
  l2 = i0;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i0 = l2;
  i1 = l1;
  i2 = l1;
  i3 = 255u;
  i2 += i3;
  i1 |= i2;
  i32_store8(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 20u;
  i0 += i1;
  l1 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  i1 = p0;
  i2 = 28u;
  i1 += i2;
  l2 = i1;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i0 = i0 > i1;
  if (i0) {
    i0 = p0;
    i1 = 36u;
    i0 += i1;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i0 = p0;
    i1 = 0u;
    i2 = 0u;
    i3 = l3;
    i4 = 7u;
    i3 &= i4;
    i4 = 2u;
    i3 += i4;
    i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, u32, u32), 0, i3, i0, i1, i2);
  }
  i0 = p0;
  i1 = 16u;
  i0 += i1;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l2;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l1;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i1 = 4u;
  i0 &= i1;
  if (i0) {
    i0 = p0;
    i1 = l1;
    i2 = 32u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4294967295u;
  } else {
    i0 = p0;
    i1 = 8u;
    i0 += i1;
    i1 = p0;
    i2 = 44u;
    i1 += i2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = p0;
    i3 = 48u;
    i2 += i3;
    i2 = i32_load(Z_envZ_memory, (u64)(i2));
    i1 += i2;
    l2 = i1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 4u;
    i0 += i1;
    i1 = l2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l1;
    i1 = 27u;
    i0 <<= (i1 & 31);
    i1 = 31u;
    i0 = (u32)((s32)i0 >> (i1 & 31));
  }
  FUNC_EPILOGUE;
  return i0;
}

static u32 f36(u32 p0, u32 p1) {
  u32 l2 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p1;
  i1 = 0u;
  i0 = i0 != i1;
  l2 = i0;
  i1 = p0;
  i2 = 3u;
  i1 &= i2;
  i2 = 0u;
  i1 = i1 != i2;
  i0 &= i1;
  if (i0) {
    L4: 
      i0 = p0;
      i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
      i0 = !(i0);
      if (i0) {goto B2;}
      i0 = p1;
      i1 = 4294967295u;
      i0 += i1;
      p1 = i0;
      i1 = 0u;
      i0 = i0 != i1;
      l2 = i0;
      i1 = p0;
      i2 = 1u;
      i1 += i2;
      p0 = i1;
      i2 = 3u;
      i1 &= i2;
      i2 = 0u;
      i1 = i1 != i2;
      i0 &= i1;
      if (i0) {goto L4;}
  }
  i0 = l2;
  i0 = !(i0);
  if (i0) {goto B1;}
  B2:;
  i0 = p0;
  i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
  i0 = !(i0);
  if (i0) {
    i0 = p1;
    i0 = !(i0);
    if (i0) {goto B1;}
    goto B0;
  }
  i0 = p1;
  i1 = 3u;
  i0 = i0 <= i1;
  if (i0) {goto B7;}
  L8: 
    i0 = p0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l2 = i0;
    i1 = 4278124287u;
    i0 += i1;
    i1 = l2;
    i2 = 2155905152u;
    i1 &= i2;
    i2 = 2155905152u;
    i1 ^= i2;
    i0 &= i1;
    i0 = !(i0);
    if (i0) {
      i0 = p0;
      i1 = 4u;
      i0 += i1;
      p0 = i0;
      i0 = p1;
      i1 = 4294967292u;
      i0 += i1;
      p1 = i0;
      i1 = 3u;
      i0 = i0 > i1;
      if (i0) {goto L8;}
      goto B7;
    }
  goto B6;
  B7:;
  i0 = p1;
  i0 = !(i0);
  if (i0) {goto B1;}
  B6:;
  L10: 
    i0 = p0;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    i0 = !(i0);
    if (i0) {goto B0;}
    i0 = p1;
    i1 = 4294967295u;
    i0 += i1;
    p1 = i0;
    i0 = !(i0);
    if (i0) {goto B1;}
    i0 = p0;
    i1 = 1u;
    i0 += i1;
    p0 = i0;
    goto L10;
  UNREACHABLE;
  B1:;
  i0 = 0u;
  p0 = i0;
  B0:;
  i0 = p0;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f37(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  i0 = g7;
  l6 = i0;
  i0 = g7;
  i1 = 16u;
  i0 += i1;
  g7 = i0;
  i0 = l6;
  l3 = i0;
  i0 = p2;
  i1 = 5268u;
  i2 = p2;
  i0 = i2 ? i0 : i1;
  l4 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  p2 = i0;
  i0 = p1;
  if (i0) {
    i0 = p0;
    i1 = l3;
    i2 = p0;
    i0 = i2 ? i0 : i1;
    l5 = i0;
    i0 = p2;
    if (i0) {
      i0 = p2;
      p0 = i0;
      i0 = 1u;
      p2 = i0;
      goto B5;
    } else {
      i0 = p1;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      p0 = i0;
      i1 = 4294967295u;
      i0 = (u32)((s32)i0 > (s32)i1);
      if (i0) {
        i0 = l5;
        i1 = p0;
        i2 = 255u;
        i1 &= i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p0;
        i1 = 0u;
        i0 = i0 != i1;
        goto B3;
      }
      i0 = p1;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      p0 = i0;
      i0 = 2460u;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      i0 = !(i0);
      if (i0) {
        i0 = l5;
        i1 = p0;
        i2 = 57343u;
        i1 &= i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = 1u;
        goto B3;
      }
      i0 = p0;
      i1 = 255u;
      i0 &= i1;
      i1 = 4294967102u;
      i0 += i1;
      p0 = i0;
      i1 = 50u;
      i0 = i0 > i1;
      if (i0) {goto B1;}
      i0 = p1;
      i1 = 1u;
      i0 += i1;
      p1 = i0;
      i0 = p0;
      i1 = 2u;
      i0 <<= (i1 & 31);
      i1 = 1024u;
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      p0 = i0;
      i0 = 0u;
      p2 = i0;
      if (i0) {goto B5;}
    }
    goto B4;
    B5:;
    i0 = p1;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    l7 = i0;
    i1 = 3u;
    i0 >>= (i1 & 31);
    l3 = i0;
    i1 = 4294967280u;
    i0 += i1;
    i1 = l3;
    i2 = p0;
    i3 = 26u;
    i2 = (u32)((s32)i2 >> (i3 & 31));
    i1 += i2;
    i0 |= i1;
    i1 = 7u;
    i0 = i0 > i1;
    if (i0) {goto B1;}
    i0 = p2;
    i1 = 4294967295u;
    i0 += i1;
    l3 = i0;
    i0 = l7;
    i1 = 4294967168u;
    i0 += i1;
    i1 = p0;
    i2 = 6u;
    i1 <<= (i2 & 31);
    i0 |= i1;
    p0 = i0;
    i1 = 0u;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {
      i0 = p1;
      p2 = i0;
      i0 = l3;
      p1 = i0;
      L10: 
        i0 = p1;
        i0 = !(i0);
        if (i0) {goto B4;}
        i0 = p2;
        i1 = 1u;
        i0 += i1;
        p2 = i0;
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
        l3 = i0;
        i1 = 192u;
        i0 &= i1;
        i1 = 128u;
        i0 = i0 != i1;
        if (i0) {goto B1;}
        i0 = p1;
        i1 = 4294967295u;
        i0 += i1;
        p1 = i0;
        i0 = l3;
        i1 = 255u;
        i0 &= i1;
        i1 = 4294967168u;
        i0 += i1;
        i1 = p0;
        i2 = 6u;
        i1 <<= (i2 & 31);
        i0 |= i1;
        p0 = i0;
        i1 = 0u;
        i0 = (u32)((s32)i0 < (s32)i1);
        if (i0) {goto L10;}
    } else {
      i0 = l3;
      p1 = i0;
    }
    i0 = l4;
    i1 = 0u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l5;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 1u;
    i1 = p1;
    i0 -= i1;
    goto B3;
    B4:;
    i0 = l4;
    i1 = p0;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4294967294u;
    B3:;
  } else {
    i0 = p2;
    if (i0) {goto B1;}
    i0 = 0u;
  }
  goto B0;
  B1:;
  i0 = l4;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 5200u;
  i1 = 84u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 4294967295u;
  B0:;
  p0 = i0;
  i0 = l6;
  g7 = i0;
  i0 = p0;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f38(u32 p0, u32 p1) {
  FUNC_PROLOGUE;
  u32 i0, i1;
  i0 = p0;
  if (i0) {
    i0 = p0;
    i1 = p1;
    i0 = f39(i0, i1);
  } else {
    i0 = 0u;
  }
  FUNC_EPILOGUE;
  return i0;
}

static u32 f39(u32 p0, u32 p1) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  if (i0) {
    i0 = p1;
    i1 = 128u;
    i0 = i0 < i1;
    if (i0) {
      i0 = p0;
      i1 = p1;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = 1u;
      goto B1;
    }
    i0 = 2460u;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i0 = !(i0);
    if (i0) {
      i0 = p1;
      i1 = 4294967168u;
      i0 &= i1;
      i1 = 57216u;
      i0 = i0 == i1;
      if (i0) {
        i0 = p0;
        i1 = p1;
        i32_store8(Z_envZ_memory, (u64)(i0), i1);
        i0 = 1u;
        goto B1;
      } else {
        i0 = 5200u;
        i1 = 84u;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = 4294967295u;
        goto B1;
      }
      UNREACHABLE;
    }
    i0 = p1;
    i1 = 2048u;
    i0 = i0 < i1;
    if (i0) {
      i0 = p0;
      i1 = p1;
      i2 = 6u;
      i1 >>= (i2 & 31);
      i2 = 192u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = p1;
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 1), i1);
      i0 = 2u;
      goto B1;
    }
    i0 = p1;
    i1 = 4294959104u;
    i0 &= i1;
    i1 = 57344u;
    i0 = i0 == i1;
    i1 = p1;
    i2 = 55296u;
    i1 = i1 < i2;
    i0 |= i1;
    if (i0) {
      i0 = p0;
      i1 = p1;
      i2 = 12u;
      i1 >>= (i2 & 31);
      i2 = 224u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = p1;
      i2 = 6u;
      i1 >>= (i2 & 31);
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 1), i1);
      i0 = p0;
      i1 = p1;
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 2), i1);
      i0 = 3u;
      goto B1;
    }
    i0 = p1;
    i1 = 4294901760u;
    i0 += i1;
    i1 = 1048576u;
    i0 = i0 < i1;
    if (i0) {
      i0 = p0;
      i1 = p1;
      i2 = 18u;
      i1 >>= (i2 & 31);
      i2 = 240u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = p1;
      i2 = 12u;
      i1 >>= (i2 & 31);
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 1), i1);
      i0 = p0;
      i1 = p1;
      i2 = 6u;
      i1 >>= (i2 & 31);
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 2), i1);
      i0 = p0;
      i1 = p1;
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 3), i1);
      i0 = 4u;
    } else {
      i0 = 5200u;
      i1 = 84u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4294967295u;
    }
    B1:;
  } else {
    i0 = 1u;
  }
  FUNC_EPILOGUE;
  return i0;
}

static void f40(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0, l9 = 0, l10 = 0, 
      l11 = 0, l12 = 0, l13 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  u64 j1;
  i0 = g7;
  l4 = i0;
  i0 = g7;
  i1 = 224u;
  i0 += i1;
  g7 = i0;
  i0 = l4;
  l5 = i0;
  i0 = l4;
  i1 = 160u;
  i0 += i1;
  l3 = i0;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0), j1);
  i0 = l3;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0 + 8), j1);
  i0 = l3;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0 + 16), j1);
  i0 = l3;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0 + 24), j1);
  i0 = l3;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0 + 32), j1);
  i0 = l4;
  i1 = 208u;
  i0 += i1;
  l6 = i0;
  i1 = p2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 0u;
  i1 = p1;
  i2 = l6;
  i3 = l4;
  i4 = 80u;
  i3 += i4;
  p2 = i3;
  i4 = l3;
  i0 = f41(i0, i1, i2, i3, i4);
  i1 = 0u;
  i0 = (u32)((s32)i0 < (s32)i1);
  if (i0) {
    i0 = 4294967295u;
  } else {
    i0 = p0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76));
    i1 = 4294967295u;
    i0 = (u32)((s32)i0 > (s32)i1);
    if (i0) {
      i0 = 1u;
    } else {
      i0 = 0u;
    }
    i0 = p0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l7 = i0;
    i0 = p0;
    i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 74));
    i1 = 1u;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {
      i0 = p0;
      i1 = l7;
      i2 = 4294967263u;
      i1 &= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    i0 = p0;
    i1 = 48u;
    i0 += i1;
    l8 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    if (i0) {
      i0 = p0;
      i1 = p1;
      i2 = l6;
      i3 = p2;
      i4 = l3;
      i0 = f41(i0, i1, i2, i3, i4);
    } else {
      i0 = p0;
      i1 = 44u;
      i0 += i1;
      l9 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l10 = i0;
      i0 = l9;
      i1 = l5;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = 28u;
      i0 += i1;
      l12 = i0;
      i1 = l5;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = 20u;
      i0 += i1;
      l11 = i0;
      i1 = l5;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l8;
      i1 = 80u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = 16u;
      i0 += i1;
      l13 = i0;
      i1 = l5;
      i2 = 80u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = p1;
      i2 = l6;
      i3 = p2;
      i4 = l3;
      i0 = f41(i0, i1, i2, i3, i4);
      i0 = l10;
      if (i0) {
        i0 = p0;
        i1 = 0u;
        i2 = 0u;
        i3 = p0;
        i3 = i32_load(Z_envZ_memory, (u64)(i3 + 36));
        i4 = 7u;
        i3 &= i4;
        i4 = 2u;
        i3 += i4;
        i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, u32, u32), 0, i3, i0, i1, i2);
        i0 = l11;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        i0 = l9;
        i1 = l10;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l8;
        i1 = 0u;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l13;
        i1 = 0u;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l12;
        i1 = 0u;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l11;
        i1 = 0u;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
    }
    i0 = p0;
    i1 = p0;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = l7;
    i3 = 32u;
    i2 &= i3;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 0u;
  }
  i0 = l4;
  g7 = i0;
  FUNC_EPILOGUE;
}

static u32 f41(u32 p0, u32 p1, u32 p2, u32 p3, u32 p4) {
  u32 l5 = 0, l6 = 0, l7 = 0, l8 = 0, l9 = 0, l10 = 0, l11 = 0, l12 = 0, 
      l13 = 0, l14 = 0, l15 = 0, l16 = 0, l17 = 0, l18 = 0, l19 = 0, l20 = 0, 
      l21 = 0, l22 = 0, l23 = 0, l24 = 0, l25 = 0, l26 = 0;
  u64 l27 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5, i6;
  u64 j0, j1, j2, j3;
  f64 d1;
  i0 = g7;
  l17 = i0;
  i0 = g7;
  i1 = 4294967232u;
  i0 -= i1;
  g7 = i0;
  i0 = l17;
  i1 = 40u;
  i0 += i1;
  l11 = i0;
  i0 = l17;
  i1 = 60u;
  i0 += i1;
  l22 = i0;
  i0 = l17;
  i1 = 56u;
  i0 += i1;
  l12 = i0;
  i1 = p1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 0u;
  i0 = i0 != i1;
  l19 = i0;
  i0 = l17;
  i1 = 40u;
  i0 += i1;
  l21 = i0;
  l20 = i0;
  i0 = l17;
  i1 = 39u;
  i0 += i1;
  l23 = i0;
  i0 = l17;
  i1 = 48u;
  i0 += i1;
  l24 = i0;
  i1 = 4u;
  i0 += i1;
  l26 = i0;
  i0 = 0u;
  p1 = i0;
  L2: 
    L4: 
      i0 = l8;
      i1 = 4294967295u;
      i0 = (u32)((s32)i0 > (s32)i1);
      if (i0) {
        i0 = p1;
        i1 = 2147483647u;
        i2 = l8;
        i1 -= i2;
        i0 = (u32)((s32)i0 > (s32)i1);
        if (i0) {
          i0 = 5200u;
          i1 = 75u;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = 4294967295u;
        } else {
          i0 = p1;
          i1 = l8;
          i0 += i1;
        }
        l8 = i0;
      }
      i0 = l12;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l10 = i0;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      l9 = i0;
      i0 = !(i0);
      if (i0) {goto B1;}
      i0 = l10;
      p1 = i0;
      L9: 
        i0 = l9;
        i1 = 24u;
        i0 <<= (i1 & 31);
        i1 = 24u;
        i0 = (u32)((s32)i0 >> (i1 & 31));
        switch (i0) {
          case 0: goto B10;
          case 1: goto B11;
          case 2: goto B11;
          case 3: goto B11;
          case 4: goto B11;
          case 5: goto B11;
          case 6: goto B11;
          case 7: goto B11;
          case 8: goto B11;
          case 9: goto B11;
          case 10: goto B11;
          case 11: goto B11;
          case 12: goto B11;
          case 13: goto B11;
          case 14: goto B11;
          case 15: goto B11;
          case 16: goto B11;
          case 17: goto B11;
          case 18: goto B11;
          case 19: goto B11;
          case 20: goto B11;
          case 21: goto B11;
          case 22: goto B11;
          case 23: goto B11;
          case 24: goto B11;
          case 25: goto B11;
          case 26: goto B11;
          case 27: goto B11;
          case 28: goto B11;
          case 29: goto B11;
          case 30: goto B11;
          case 31: goto B11;
          case 32: goto B11;
          case 33: goto B11;
          case 34: goto B11;
          case 35: goto B11;
          case 36: goto B11;
          case 37: goto B8;
          default: goto B11;
        }
        B11:;
        i0 = l12;
        i1 = p1;
        i2 = 1u;
        i1 += i2;
        p1 = i1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p1;
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
        l9 = i0;
        goto L9;
        B10:;
      goto B7;
      B8:;
      i0 = p1;
      l9 = i0;
      L12: 
        i0 = p1;
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 1));
        i1 = 37u;
        i0 = i0 != i1;
        if (i0) {
          i0 = l9;
          p1 = i0;
          goto B7;
        }
        i0 = l9;
        i1 = 1u;
        i0 += i1;
        l9 = i0;
        i0 = l12;
        i1 = p1;
        i2 = 2u;
        i1 += i2;
        p1 = i1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p1;
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
        i1 = 37u;
        i0 = i0 == i1;
        if (i0) {goto L12;}
        i0 = l9;
      p1 = i0;
      B7:;
      i0 = p1;
      i1 = l10;
      i0 -= i1;
      p1 = i0;
      i0 = l19;
      if (i0) {
        i0 = p0;
        i1 = l10;
        i2 = p1;
        f42(i0, i1, i2);
      }
      i0 = p1;
      if (i0) {goto L4;}
    i0 = l12;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 1));
    i0 = f29(i0);
    i0 = !(i0);
    l9 = i0;
    i0 = l12;
    i1 = l12;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    p1 = i1;
    i2 = l9;
    if (i2) {
      i2 = 4294967295u;
      l14 = i2;
      i2 = 1u;
    } else {
      i2 = p1;
      i2 = i32_load8_s(Z_envZ_memory, (u64)(i2 + 2));
      i3 = 36u;
      i2 = i2 == i3;
      if (i2) {
        i2 = p1;
        i2 = i32_load8_s(Z_envZ_memory, (u64)(i2 + 1));
        i3 = 4294967248u;
        i2 += i3;
        l14 = i2;
        i2 = 1u;
        l5 = i2;
        i2 = 3u;
      } else {
        i2 = 4294967295u;
        l14 = i2;
        i2 = 1u;
      }
    }
    i1 += i2;
    p1 = i1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
    l6 = i0;
    i1 = 4294967264u;
    i0 += i1;
    l9 = i0;
    i1 = 31u;
    i0 = i0 > i1;
    i1 = 1u;
    i2 = l9;
    i1 <<= (i2 & 31);
    i2 = 75913u;
    i1 &= i2;
    i1 = !(i1);
    i0 |= i1;
    if (i0) {
      i0 = 0u;
      l9 = i0;
    } else {
      i0 = 0u;
      l6 = i0;
      L18: 
        i0 = l6;
        i1 = 1u;
        i2 = l9;
        i1 <<= (i2 & 31);
        i0 |= i1;
        l9 = i0;
        i0 = l12;
        i1 = p1;
        i2 = 1u;
        i1 += i2;
        p1 = i1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p1;
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
        l6 = i0;
        i1 = 4294967264u;
        i0 += i1;
        l7 = i0;
        i1 = 31u;
        i0 = i0 > i1;
        i1 = 1u;
        i2 = l7;
        i1 <<= (i2 & 31);
        i2 = 75913u;
        i1 &= i2;
        i1 = !(i1);
        i0 |= i1;
        i0 = !(i0);
        if (i0) {
          i0 = l9;
          l6 = i0;
          i0 = l7;
          l9 = i0;
          goto L18;
        }
    }
    i0 = l6;
    i1 = 255u;
    i0 &= i1;
    i1 = 42u;
    i0 = i0 == i1;
    if (i0) {
      i0 = l12;
      i1 = p1;
      i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 1));
      i1 = f29(i1);
      i1 = !(i1);
      if (i1) {goto B22;}
      i1 = l12;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      l7 = i1;
      i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 2));
      i2 = 36u;
      i1 = i1 != i2;
      if (i1) {goto B22;}
      i1 = l7;
      i2 = 1u;
      i1 += i2;
      p1 = i1;
      i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
      i2 = 4294967248u;
      i1 += i2;
      i2 = 2u;
      i1 <<= (i2 & 31);
      i2 = p4;
      i1 += i2;
      i2 = 10u;
      i32_store(Z_envZ_memory, (u64)(i1), i2);
      i1 = p1;
      i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
      i2 = 4294967248u;
      i1 += i2;
      i2 = 3u;
      i1 <<= (i2 & 31);
      i2 = p3;
      i1 += i2;
      j1 = i64_load(Z_envZ_memory, (u64)(i1));
      i1 = (u32)(j1);
      p1 = i1;
      i1 = 1u;
      l6 = i1;
      i1 = l7;
      i2 = 3u;
      i1 += i2;
      goto B21;
      B22:;
      i1 = l5;
      if (i1) {
        i1 = 4294967295u;
        l8 = i1;
        goto B3;
      }
      i1 = l19;
      if (i1) {
        i1 = p2;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i2 = 3u;
        i1 += i2;
        i2 = 4294967292u;
        i1 &= i2;
        l5 = i1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        p1 = i1;
        i1 = p2;
        i2 = l5;
        i3 = 4u;
        i2 += i3;
        i32_store(Z_envZ_memory, (u64)(i1), i2);
      } else {
        i1 = 0u;
        p1 = i1;
      }
      i1 = 0u;
      l6 = i1;
      i1 = l12;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 1u;
      i1 += i2;
      B21:;
      l5 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 0u;
      i1 = p1;
      i0 -= i1;
      i1 = p1;
      i2 = p1;
      i3 = 0u;
      i2 = (u32)((s32)i2 < (s32)i3);
      p1 = i2;
      i0 = i2 ? i0 : i1;
      l15 = i0;
      i0 = l9;
      i1 = 8192u;
      i0 |= i1;
      i1 = l9;
      i2 = p1;
      i0 = i2 ? i0 : i1;
      l16 = i0;
      i0 = l6;
      l9 = i0;
    } else {
      i0 = l12;
      i0 = f43(i0);
      l15 = i0;
      i1 = 0u;
      i0 = (u32)((s32)i0 < (s32)i1);
      if (i0) {
        i0 = 4294967295u;
        l8 = i0;
        goto B3;
      }
      i0 = l9;
      l16 = i0;
      i0 = l5;
      l9 = i0;
      i0 = l12;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l5 = i0;
    }
    i0 = l5;
    i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
    i1 = 46u;
    i0 = i0 == i1;
    if (i0) {
      i0 = l5;
      i1 = 1u;
      i0 += i1;
      p1 = i0;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      i1 = 42u;
      i0 = i0 != i1;
      if (i0) {
        i0 = l12;
        i1 = p1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l12;
        i0 = f43(i0);
        p1 = i0;
        i0 = l12;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l5 = i0;
        goto B27;
      }
      i0 = l5;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 2));
      i0 = f29(i0);
      if (i0) {
        i0 = l12;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l5 = i0;
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 3));
        i1 = 36u;
        i0 = i0 == i1;
        if (i0) {
          i0 = l5;
          i1 = 2u;
          i0 += i1;
          p1 = i0;
          i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
          i1 = 4294967248u;
          i0 += i1;
          i1 = 2u;
          i0 <<= (i1 & 31);
          i1 = p4;
          i0 += i1;
          i1 = 10u;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = p1;
          i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
          i1 = 4294967248u;
          i0 += i1;
          i1 = 3u;
          i0 <<= (i1 & 31);
          i1 = p3;
          i0 += i1;
          j0 = i64_load(Z_envZ_memory, (u64)(i0));
          i0 = (u32)(j0);
          p1 = i0;
          i0 = l12;
          i1 = l5;
          i2 = 4u;
          i1 += i2;
          l5 = i1;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          goto B27;
        }
      }
      i0 = l9;
      if (i0) {
        i0 = 4294967295u;
        l8 = i0;
        goto B3;
      }
      i0 = l19;
      if (i0) {
        i0 = p2;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        i1 = 3u;
        i0 += i1;
        i1 = 4294967292u;
        i0 &= i1;
        l5 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        p1 = i0;
        i0 = p2;
        i1 = l5;
        i2 = 4u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      } else {
        i0 = 0u;
        p1 = i0;
      }
      i0 = l12;
      i1 = l12;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 2u;
      i1 += i2;
      l5 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      B27:;
    } else {
      i0 = 4294967295u;
      p1 = i0;
    }
    i0 = 0u;
    l13 = i0;
    L33: 
      i0 = l5;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      i1 = 4294967231u;
      i0 += i1;
      i1 = 57u;
      i0 = i0 > i1;
      if (i0) {
        i0 = 4294967295u;
        l8 = i0;
        goto B3;
      }
      i0 = l12;
      i1 = l5;
      i2 = 1u;
      i1 += i2;
      l6 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l5;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      i1 = l13;
      i2 = 58u;
      i1 *= i2;
      i0 += i1;
      i1 = 1439u;
      i0 += i1;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      l7 = i0;
      i1 = 255u;
      i0 &= i1;
      l5 = i0;
      i1 = 4294967295u;
      i0 += i1;
      i1 = 8u;
      i0 = i0 < i1;
      if (i0) {
        i0 = l5;
        l13 = i0;
        i0 = l6;
        l5 = i0;
        goto L33;
      }
    i0 = l7;
    i0 = !(i0);
    if (i0) {
      i0 = 4294967295u;
      l8 = i0;
      goto B3;
    }
    i0 = l14;
    i1 = 4294967295u;
    i0 = (u32)((s32)i0 > (s32)i1);
    l18 = i0;
    i0 = l7;
    i1 = 19u;
    i0 = i0 == i1;
    if (i0) {
      i0 = l18;
      if (i0) {
        i0 = 4294967295u;
        l8 = i0;
        goto B3;
      }
    } else {
      i0 = l18;
      if (i0) {
        i0 = l14;
        i1 = 2u;
        i0 <<= (i1 & 31);
        i1 = p4;
        i0 += i1;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l11;
        i1 = l14;
        i2 = 3u;
        i1 <<= (i2 & 31);
        i2 = p3;
        i1 += i2;
        j1 = i64_load(Z_envZ_memory, (u64)(i1));
        i64_store(Z_envZ_memory, (u64)(i0), j1);
        goto B41;
      }
      i0 = l19;
      i0 = !(i0);
      if (i0) {
        i0 = 0u;
        l8 = i0;
        goto B3;
      }
      i0 = l11;
      i1 = l5;
      i2 = p2;
      f44(i0, i1, i2);
      i0 = l12;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l6 = i0;
      goto B38;
      B41:;
    }
    i0 = l19;
    if (i0) {goto B38;}
    i0 = 0u;
    p1 = i0;
    goto B37;
    B38:;
    i0 = l16;
    i1 = 4294901759u;
    i0 &= i1;
    l7 = i0;
    i1 = l16;
    i2 = l16;
    i3 = 8192u;
    i2 &= i3;
    i0 = i2 ? i0 : i1;
    l5 = i0;
    i0 = l6;
    i1 = 4294967295u;
    i0 += i1;
    i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
    l6 = i0;
    i1 = 4294967263u;
    i0 &= i1;
    i1 = l6;
    i2 = l6;
    i3 = 15u;
    i2 &= i3;
    i3 = 3u;
    i2 = i2 == i3;
    i3 = l13;
    i4 = 0u;
    i3 = i3 != i4;
    i2 &= i3;
    i0 = i2 ? i0 : i1;
    l6 = i0;
    i1 = 65u;
    i0 -= i1;
    switch (i0) {
      case 0: goto B51;
      case 1: goto B50;
      case 2: goto B53;
      case 3: goto B50;
      case 4: goto B51;
      case 5: goto B51;
      case 6: goto B51;
      case 7: goto B50;
      case 8: goto B50;
      case 9: goto B50;
      case 10: goto B50;
      case 11: goto B50;
      case 12: goto B50;
      case 13: goto B50;
      case 14: goto B50;
      case 15: goto B50;
      case 16: goto B50;
      case 17: goto B50;
      case 18: goto B52;
      case 19: goto B50;
      case 20: goto B50;
      case 21: goto B50;
      case 22: goto B50;
      case 23: goto B49;
      case 24: goto B50;
      case 25: goto B50;
      case 26: goto B50;
      case 27: goto B50;
      case 28: goto B50;
      case 29: goto B50;
      case 30: goto B50;
      case 31: goto B50;
      case 32: goto B51;
      case 33: goto B50;
      case 34: goto B55;
      case 35: goto B57;
      case 36: goto B51;
      case 37: goto B51;
      case 38: goto B51;
      case 39: goto B50;
      case 40: goto B57;
      case 41: goto B50;
      case 42: goto B50;
      case 43: goto B50;
      case 44: goto B50;
      case 45: goto B60;
      case 46: goto B58;
      case 47: goto B59;
      case 48: goto B50;
      case 49: goto B50;
      case 50: goto B54;
      case 51: goto B50;
      case 52: goto B56;
      case 53: goto B50;
      case 54: goto B50;
      case 55: goto B49;
      default: goto B50;
    }
    B60:;
    i0 = l13;
    i1 = 255u;
    i0 &= i1;
    i1 = 24u;
    i0 <<= (i1 & 31);
    i1 = 24u;
    i0 = (u32)((s32)i0 >> (i1 & 31));
    switch (i0) {
      case 0: goto B68;
      case 1: goto B67;
      case 2: goto B66;
      case 3: goto B65;
      case 4: goto B64;
      case 5: goto B61;
      case 6: goto B63;
      case 7: goto B62;
      default: goto B61;
    }
    B68:;
    i0 = l11;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = l8;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 0u;
    p1 = i0;
    goto B37;
    B67:;
    i0 = l11;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = l8;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 0u;
    p1 = i0;
    goto B37;
    B66:;
    i0 = l11;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = l8;
    j1 = (u64)(s64)(s32)(i1);
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    i0 = 0u;
    p1 = i0;
    goto B37;
    B65:;
    i0 = l11;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = l8;
    i32_store16(Z_envZ_memory, (u64)(i0), i1);
    i0 = 0u;
    p1 = i0;
    goto B37;
    B64:;
    i0 = l11;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = l8;
    i32_store8(Z_envZ_memory, (u64)(i0), i1);
    i0 = 0u;
    p1 = i0;
    goto B37;
    B63:;
    i0 = l11;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = l8;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 0u;
    p1 = i0;
    goto B37;
    B62:;
    i0 = l11;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = l8;
    j1 = (u64)(s64)(s32)(i1);
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    i0 = 0u;
    p1 = i0;
    goto B37;
    B61:;
    i0 = 0u;
    p1 = i0;
    goto B37;
    B59:;
    i0 = 120u;
    l6 = i0;
    i0 = p1;
    i1 = 8u;
    i2 = p1;
    i3 = 8u;
    i2 = i2 > i3;
    i0 = i2 ? i0 : i1;
    p1 = i0;
    i0 = l5;
    i1 = 8u;
    i0 |= i1;
    l5 = i0;
    goto B49;
    B58:;
    i0 = 0u;
    l10 = i0;
    i0 = 2553u;
    l7 = i0;
    i0 = p1;
    i1 = l20;
    i2 = l11;
    j2 = i64_load(Z_envZ_memory, (u64)(i2));
    l27 = j2;
    i3 = l21;
    i2 = f46(j2, i3);
    l13 = i2;
    i1 -= i2;
    l6 = i1;
    i2 = 1u;
    i1 += i2;
    i2 = l5;
    i3 = 8u;
    i2 &= i3;
    i2 = !(i2);
    i3 = p1;
    i4 = l6;
    i3 = (u32)((s32)i3 > (s32)i4);
    i2 |= i3;
    i0 = i2 ? i0 : i1;
    p1 = i0;
    goto B46;
    B57:;
    i0 = l11;
    j0 = i64_load(Z_envZ_memory, (u64)(i0));
    l27 = j0;
    j1 = 0ull;
    i0 = (u64)((s64)j0 < (s64)j1);
    if (i0) {
      i0 = l11;
      j1 = 0ull;
      j2 = l27;
      j1 -= j2;
      l27 = j1;
      i64_store(Z_envZ_memory, (u64)(i0), j1);
      i0 = 1u;
      l10 = i0;
      i0 = 2553u;
      l7 = i0;
      goto B48;
    } else {
      i0 = l5;
      i1 = 2049u;
      i0 &= i1;
      i1 = 0u;
      i0 = i0 != i1;
      l10 = i0;
      i0 = 2554u;
      i1 = 2555u;
      i2 = 2553u;
      i3 = l5;
      i4 = 1u;
      i3 &= i4;
      i1 = i3 ? i1 : i2;
      i2 = l5;
      i3 = 2048u;
      i2 &= i3;
      i0 = i2 ? i0 : i1;
      l7 = i0;
      goto B48;
    }
    UNREACHABLE;
    B56:;
    i0 = 0u;
    l10 = i0;
    i0 = 2553u;
    l7 = i0;
    i0 = l11;
    j0 = i64_load(Z_envZ_memory, (u64)(i0));
    l27 = j0;
    goto B48;
    B55:;
    i0 = l23;
    i1 = l11;
    j1 = i64_load(Z_envZ_memory, (u64)(i1));
    i64_store8(Z_envZ_memory, (u64)(i0), j1);
    i0 = l23;
    l6 = i0;
    i0 = 0u;
    l10 = i0;
    i0 = 2553u;
    l16 = i0;
    i0 = 1u;
    l13 = i0;
    i0 = l7;
    l5 = i0;
    i0 = l20;
    p1 = i0;
    goto B44;
    B54:;
    i0 = l11;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l5 = i0;
    i1 = 2563u;
    i2 = l5;
    i0 = i2 ? i0 : i1;
    l14 = i0;
    i1 = p1;
    i0 = f36(i0, i1);
    l18 = i0;
    i0 = !(i0);
    l25 = i0;
    i0 = 0u;
    l10 = i0;
    i0 = 2553u;
    l16 = i0;
    i0 = p1;
    i1 = l18;
    i2 = l14;
    l6 = i2;
    i1 -= i2;
    i2 = l25;
    i0 = i2 ? i0 : i1;
    l13 = i0;
    i0 = l7;
    l5 = i0;
    i0 = p1;
    i1 = l6;
    i0 += i1;
    i1 = l18;
    i2 = l25;
    i0 = i2 ? i0 : i1;
    p1 = i0;
    goto B44;
    B53:;
    i0 = l24;
    i1 = l11;
    j1 = i64_load(Z_envZ_memory, (u64)(i1));
    i64_store32(Z_envZ_memory, (u64)(i0), j1);
    i0 = l26;
    i1 = 0u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l11;
    i1 = l24;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4294967295u;
    l10 = i0;
    goto B47;
    B52:;
    i0 = p1;
    if (i0) {
      i0 = p1;
      l10 = i0;
      goto B47;
    } else {
      i0 = p0;
      i1 = 32u;
      i2 = l15;
      i3 = 0u;
      i4 = l5;
      f48(i0, i1, i2, i3, i4);
      i0 = 0u;
      p1 = i0;
      goto B45;
    }
    UNREACHABLE;
    B51:;
    i0 = p0;
    i1 = l11;
    d1 = f64_load(Z_envZ_memory, (u64)(i1));
    i2 = l15;
    i3 = p1;
    i4 = l5;
    i5 = l6;
    i0 = f49(i0, d1, i2, i3, i4, i5);
    p1 = i0;
    goto B37;
    B50:;
    i0 = l10;
    l6 = i0;
    i0 = 0u;
    l10 = i0;
    i0 = 2553u;
    l16 = i0;
    i0 = p1;
    l13 = i0;
    i0 = l20;
    p1 = i0;
    goto B44;
    B49:;
    i0 = l5;
    i1 = 8u;
    i0 &= i1;
    i0 = !(i0);
    i1 = l11;
    j1 = i64_load(Z_envZ_memory, (u64)(i1));
    l27 = j1;
    j2 = 0ull;
    i1 = j1 == j2;
    i0 |= i1;
    l7 = i0;
    j0 = l27;
    i1 = l21;
    i2 = l6;
    i3 = 32u;
    i2 &= i3;
    i0 = f45(j0, i1, i2);
    l13 = i0;
    i0 = 0u;
    i1 = 2u;
    i2 = l7;
    i0 = i2 ? i0 : i1;
    l10 = i0;
    i0 = 2553u;
    i1 = l6;
    i2 = 4u;
    i1 >>= (i2 & 31);
    i2 = 2553u;
    i1 += i2;
    i2 = l7;
    i0 = i2 ? i0 : i1;
    l7 = i0;
    goto B46;
    B48:;
    j0 = l27;
    i1 = l21;
    i0 = f47(j0, i1);
    l13 = i0;
    goto B46;
    B47:;
    i0 = l11;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l6 = i0;
    i0 = 0u;
    p1 = i0;
    L73: 
      i0 = l6;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l7 = i0;
      if (i0) {
        i0 = l22;
        i1 = l7;
        i0 = f38(i0, i1);
        l7 = i0;
        i1 = 0u;
        i0 = (u32)((s32)i0 < (s32)i1);
        l13 = i0;
        i1 = l7;
        i2 = l10;
        i3 = p1;
        i2 -= i3;
        i1 = i1 > i2;
        i0 |= i1;
        if (i0) {goto B72;}
        i0 = l6;
        i1 = 4u;
        i0 += i1;
        l6 = i0;
        i0 = l10;
        i1 = p1;
        i2 = l7;
        i1 += i2;
        p1 = i1;
        i0 = i0 > i1;
        if (i0) {goto L73;}
      }
    goto B71;
    B72:;
    i0 = l13;
    if (i0) {
      i0 = 4294967295u;
      l8 = i0;
      goto B3;
    }
    B71:;
    i0 = p0;
    i1 = 32u;
    i2 = l15;
    i3 = p1;
    i4 = l5;
    f48(i0, i1, i2, i3, i4);
    i0 = p1;
    if (i0) {
      i0 = l11;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l6 = i0;
      i0 = 0u;
      l10 = i0;
      L77: 
        i0 = l6;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l7 = i0;
        i0 = !(i0);
        if (i0) {goto B45;}
        i0 = l10;
        i1 = l22;
        i2 = l7;
        i1 = f38(i1, i2);
        l7 = i1;
        i0 += i1;
        l10 = i0;
        i1 = p1;
        i0 = (u32)((s32)i0 > (s32)i1);
        if (i0) {goto B45;}
        i0 = l6;
        i1 = 4u;
        i0 += i1;
        l6 = i0;
        i0 = p0;
        i1 = l22;
        i2 = l7;
        f42(i0, i1, i2);
        i0 = l10;
        i1 = p1;
        i0 = i0 < i1;
        if (i0) {goto L77;}
      goto B45;
    } else {
      i0 = 0u;
      p1 = i0;
      goto B45;
    }
    UNREACHABLE;
    B46:;
    i0 = l13;
    i1 = l21;
    j2 = l27;
    j3 = 0ull;
    i2 = j2 != j3;
    l14 = i2;
    i3 = p1;
    i4 = 0u;
    i3 = i3 != i4;
    i2 |= i3;
    l18 = i2;
    i0 = i2 ? i0 : i1;
    l6 = i0;
    i0 = l7;
    l16 = i0;
    i0 = p1;
    i1 = l20;
    i2 = l13;
    i1 -= i2;
    i2 = l14;
    i3 = 1u;
    i2 ^= i3;
    i3 = 1u;
    i2 &= i3;
    i1 += i2;
    l7 = i1;
    i2 = p1;
    i3 = l7;
    i2 = (u32)((s32)i2 > (s32)i3);
    i0 = i2 ? i0 : i1;
    i1 = 0u;
    i2 = l18;
    i0 = i2 ? i0 : i1;
    l13 = i0;
    i0 = l5;
    i1 = 4294901759u;
    i0 &= i1;
    i1 = l5;
    i2 = p1;
    i3 = 4294967295u;
    i2 = (u32)((s32)i2 > (s32)i3);
    i0 = i2 ? i0 : i1;
    l5 = i0;
    i0 = l20;
    p1 = i0;
    goto B44;
    B45:;
    i0 = p0;
    i1 = 32u;
    i2 = l15;
    i3 = p1;
    i4 = l5;
    i5 = 8192u;
    i4 ^= i5;
    f48(i0, i1, i2, i3, i4);
    i0 = l15;
    i1 = p1;
    i2 = l15;
    i3 = p1;
    i2 = (u32)((s32)i2 > (s32)i3);
    i0 = i2 ? i0 : i1;
    p1 = i0;
    goto B37;
    B44:;
    i0 = p0;
    i1 = 32u;
    i2 = l10;
    i3 = p1;
    i4 = l6;
    i3 -= i4;
    l14 = i3;
    i4 = l13;
    i5 = l13;
    i6 = l14;
    i5 = (u32)((s32)i5 < (s32)i6);
    i3 = i5 ? i3 : i4;
    l13 = i3;
    i2 += i3;
    l7 = i2;
    i3 = l15;
    i4 = l15;
    i5 = l7;
    i4 = (u32)((s32)i4 < (s32)i5);
    i2 = i4 ? i2 : i3;
    p1 = i2;
    i3 = l7;
    i4 = l5;
    f48(i0, i1, i2, i3, i4);
    i0 = p0;
    i1 = l16;
    i2 = l10;
    f42(i0, i1, i2);
    i0 = p0;
    i1 = 48u;
    i2 = p1;
    i3 = l7;
    i4 = l5;
    i5 = 65536u;
    i4 ^= i5;
    f48(i0, i1, i2, i3, i4);
    i0 = p0;
    i1 = 48u;
    i2 = l13;
    i3 = l14;
    i4 = 0u;
    f48(i0, i1, i2, i3, i4);
    i0 = p0;
    i1 = l6;
    i2 = l14;
    f42(i0, i1, i2);
    i0 = p0;
    i1 = 32u;
    i2 = p1;
    i3 = l7;
    i4 = l5;
    i5 = 8192u;
    i4 ^= i5;
    f48(i0, i1, i2, i3, i4);
    B37:;
    i0 = l9;
    l5 = i0;
    goto L2;
    B3:;
  goto B0;
  B1:;
  i0 = p0;
  i0 = !(i0);
  if (i0) {
    i0 = l5;
    if (i0) {
      i0 = 1u;
      p0 = i0;
      L80: 
        i0 = p0;
        i1 = 2u;
        i0 <<= (i1 & 31);
        i1 = p4;
        i0 += i1;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        p1 = i0;
        if (i0) {
          i0 = p0;
          i1 = 3u;
          i0 <<= (i1 & 31);
          i1 = p3;
          i0 += i1;
          i1 = p1;
          i2 = p2;
          f44(i0, i1, i2);
          i0 = p0;
          i1 = 1u;
          i0 += i1;
          p0 = i0;
          i1 = 10u;
          i0 = i0 < i1;
          if (i0) {goto L80;}
          i0 = 1u;
          l8 = i0;
          goto B0;
        }
      L82: 
        i0 = p0;
        i1 = 2u;
        i0 <<= (i1 & 31);
        i1 = p4;
        i0 += i1;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        if (i0) {
          i0 = 4294967295u;
          l8 = i0;
          goto B0;
        }
        i0 = p0;
        i1 = 1u;
        i0 += i1;
        p0 = i0;
        i1 = 10u;
        i0 = i0 < i1;
        if (i0) {goto L82;}
        i0 = 1u;
    } else {
      i0 = 0u;
    }
    l8 = i0;
  }
  B0:;
  i0 = l17;
  g7 = i0;
  i0 = l8;
  FUNC_EPILOGUE;
  return i0;
}

static void f42(u32 p0, u32 p1, u32 p2) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  i1 = 32u;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {
    i0 = p1;
    i1 = p2;
    i2 = p0;
    f51(i0, i1, i2);
  }
  FUNC_EPILOGUE;
}

static u32 f43(u32 p0) {
  u32 l1 = 0, l2 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
  i0 = f29(i0);
  if (i0) {
    L1: 
      i0 = p0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l2 = i0;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      i1 = l1;
      i2 = 10u;
      i1 *= i2;
      i2 = 4294967248u;
      i1 += i2;
      i0 += i1;
      l1 = i0;
      i0 = p0;
      i1 = l2;
      i2 = 1u;
      i1 += i2;
      l2 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l2;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      i0 = f29(i0);
      if (i0) {goto L1;}
  }
  i0 = l1;
  FUNC_EPILOGUE;
  return i0;
}

static void f44(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0;
  u64 l4 = 0;
  f64 l5 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  u64 j0, j1;
  f64 d0, d1;
  i0 = p1;
  i1 = 20u;
  i0 = i0 <= i1;
  if (i0) {
    i0 = p1;
    i1 = 9u;
    i0 -= i1;
    switch (i0) {
      case 0: goto B11;
      case 1: goto B10;
      case 2: goto B9;
      case 3: goto B8;
      case 4: goto B7;
      case 5: goto B6;
      case 6: goto B5;
      case 7: goto B4;
      case 8: goto B3;
      case 9: goto B2;
      default: goto B1;
    }
    B11:;
    i0 = p2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = 3u;
    i0 += i1;
    i1 = 4294967292u;
    i0 &= i1;
    p1 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i0 = p2;
    i1 = p1;
    i2 = 4u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l3;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    goto B1;
    B10:;
    i0 = p2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = 3u;
    i0 += i1;
    i1 = 4294967292u;
    i0 &= i1;
    p1 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i0 = p2;
    i1 = p1;
    i2 = 4u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l3;
    j1 = (u64)(s64)(s32)(i1);
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    goto B1;
    B9:;
    i0 = p2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = 3u;
    i0 += i1;
    i1 = 4294967292u;
    i0 &= i1;
    p1 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i0 = p2;
    i1 = p1;
    i2 = 4u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l3;
    j1 = (u64)(i1);
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    goto B1;
    B8:;
    i0 = p2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = 7u;
    i0 += i1;
    i1 = 4294967288u;
    i0 &= i1;
    p1 = i0;
    j0 = i64_load(Z_envZ_memory, (u64)(i0));
    l4 = j0;
    i0 = p2;
    i1 = p1;
    i2 = 8u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    j1 = l4;
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    goto B1;
    B7:;
    i0 = p2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = 3u;
    i0 += i1;
    i1 = 4294967292u;
    i0 &= i1;
    p1 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i0 = p2;
    i1 = p1;
    i2 = 4u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l3;
    i2 = 65535u;
    i1 &= i2;
    i2 = 16u;
    i1 <<= (i2 & 31);
    i2 = 16u;
    i1 = (u32)((s32)i1 >> (i2 & 31));
    j1 = (u64)(s64)(s32)(i1);
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    goto B1;
    B6:;
    i0 = p2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = 3u;
    i0 += i1;
    i1 = 4294967292u;
    i0 &= i1;
    p1 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i0 = p2;
    i1 = p1;
    i2 = 4u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l3;
    i2 = 65535u;
    i1 &= i2;
    j1 = (u64)(i1);
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    goto B1;
    B5:;
    i0 = p2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = 3u;
    i0 += i1;
    i1 = 4294967292u;
    i0 &= i1;
    p1 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i0 = p2;
    i1 = p1;
    i2 = 4u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l3;
    i2 = 255u;
    i1 &= i2;
    i2 = 24u;
    i1 <<= (i2 & 31);
    i2 = 24u;
    i1 = (u32)((s32)i1 >> (i2 & 31));
    j1 = (u64)(s64)(s32)(i1);
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    goto B1;
    B4:;
    i0 = p2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = 3u;
    i0 += i1;
    i1 = 4294967292u;
    i0 &= i1;
    p1 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i0 = p2;
    i1 = p1;
    i2 = 4u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = l3;
    i2 = 255u;
    i1 &= i2;
    j1 = (u64)(i1);
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    goto B1;
    B3:;
    i0 = p2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = 7u;
    i0 += i1;
    i1 = 4294967288u;
    i0 &= i1;
    p1 = i0;
    d0 = f64_load(Z_envZ_memory, (u64)(i0));
    l5 = d0;
    i0 = p2;
    i1 = p1;
    i2 = 8u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    d1 = l5;
    f64_store(Z_envZ_memory, (u64)(i0), d1);
    goto B1;
    B2:;
    i0 = p2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = 7u;
    i0 += i1;
    i1 = 4294967288u;
    i0 &= i1;
    p1 = i0;
    d0 = f64_load(Z_envZ_memory, (u64)(i0));
    l5 = d0;
    i0 = p2;
    i1 = p1;
    i2 = 8u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    d1 = l5;
    f64_store(Z_envZ_memory, (u64)(i0), d1);
    B1:;
  }
  FUNC_EPILOGUE;
}

static u32 f45(u64 p0, u32 p1, u32 p2) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  u64 j0, j1, j2;
  j0 = p0;
  j1 = 0ull;
  i0 = j0 != j1;
  if (i0) {
    L1: 
      i0 = p1;
      i1 = 4294967295u;
      i0 += i1;
      p1 = i0;
      i1 = p2;
      j2 = p0;
      i2 = (u32)(j2);
      i3 = 15u;
      i2 &= i3;
      i3 = 1968u;
      i2 += i3;
      i2 = i32_load8_u(Z_envZ_memory, (u64)(i2));
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      j0 = p0;
      j1 = 4ull;
      j0 >>= (j1 & 63);
      p0 = j0;
      j1 = 0ull;
      i0 = j0 != j1;
      if (i0) {goto L1;}
  }
  i0 = p1;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f46(u64 p0, u32 p1) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  u64 j0, j1;
  j0 = p0;
  j1 = 0ull;
  i0 = j0 != j1;
  if (i0) {
    L1: 
      i0 = p1;
      i1 = 4294967295u;
      i0 += i1;
      p1 = i0;
      j1 = p0;
      i1 = (u32)(j1);
      i2 = 7u;
      i1 &= i2;
      i2 = 48u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      j0 = p0;
      j1 = 3ull;
      j0 >>= (j1 & 63);
      p0 = j0;
      j1 = 0ull;
      i0 = j0 != j1;
      if (i0) {goto L1;}
  }
  i0 = p1;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f47(u64 p0, u32 p1) {
  u32 l2 = 0, l3 = 0;
  u64 l4 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  u64 j0, j1, j2, j3;
  j0 = p0;
  i0 = (u32)(j0);
  l2 = i0;
  j0 = p0;
  j1 = 4294967295ull;
  i0 = j0 > j1;
  if (i0) {
    L1: 
      i0 = p1;
      i1 = 4294967295u;
      i0 += i1;
      p1 = i0;
      j1 = p0;
      j2 = p0;
      j3 = 10ull;
      j2 = DIV_U(j2, j3);
      l4 = j2;
      j3 = 10ull;
      j2 *= j3;
      j1 -= j2;
      i1 = (u32)(j1);
      i2 = 255u;
      i1 &= i2;
      i2 = 48u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      j0 = p0;
      j1 = 42949672959ull;
      i0 = j0 > j1;
      if (i0) {
        j0 = l4;
        p0 = j0;
        goto L1;
      }
    j0 = l4;
    i0 = (u32)(j0);
    l2 = i0;
  }
  i0 = l2;
  if (i0) {
    L4: 
      i0 = p1;
      i1 = 4294967295u;
      i0 += i1;
      p1 = i0;
      i1 = l2;
      i2 = l2;
      i3 = 10u;
      i2 = DIV_U(i2, i3);
      l3 = i2;
      i3 = 10u;
      i2 *= i3;
      i1 -= i2;
      i2 = 48u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = l2;
      i1 = 10u;
      i0 = i0 >= i1;
      if (i0) {
        i0 = l3;
        l2 = i0;
        goto L4;
      }
  }
  i0 = p1;
  FUNC_EPILOGUE;
  return i0;
}

static void f48(u32 p0, u32 p1, u32 p2, u32 p3, u32 p4) {
  u32 l5 = 0, l6 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5;
  i0 = g7;
  l6 = i0;
  i0 = g7;
  i1 = 256u;
  i0 += i1;
  g7 = i0;
  i0 = l6;
  l5 = i0;
  i0 = p4;
  i1 = 73728u;
  i0 &= i1;
  i0 = !(i0);
  i1 = p2;
  i2 = p3;
  i1 = (u32)((s32)i1 > (s32)i2);
  i0 &= i1;
  if (i0) {
    i0 = l5;
    i1 = p1;
    i2 = 24u;
    i1 <<= (i2 & 31);
    i2 = 24u;
    i1 = (u32)((s32)i1 >> (i2 & 31));
    i2 = p2;
    i3 = p3;
    i2 -= i3;
    p1 = i2;
    i3 = 256u;
    i4 = p1;
    i5 = 256u;
    i4 = i4 < i5;
    i2 = i4 ? i2 : i3;
    i0 = _memset(i0, i1, i2);
    i0 = p1;
    i1 = 255u;
    i0 = i0 > i1;
    if (i0) {
      i0 = p2;
      i1 = p3;
      i0 -= i1;
      p2 = i0;
      L2: 
        i0 = p0;
        i1 = l5;
        i2 = 256u;
        f42(i0, i1, i2);
        i0 = p1;
        i1 = 4294967040u;
        i0 += i1;
        p1 = i0;
        i1 = 255u;
        i0 = i0 > i1;
        if (i0) {goto L2;}
      i0 = p2;
      i1 = 255u;
      i0 &= i1;
      p1 = i0;
    }
    i0 = p0;
    i1 = l5;
    i2 = p1;
    f42(i0, i1, i2);
  }
  i0 = l6;
  g7 = i0;
  FUNC_EPILOGUE;
}

static u32 f49(u32 p0, f64 p1, u32 p2, u32 p3, u32 p4, u32 p5) {
  u32 l6 = 0, l7 = 0, l8 = 0, l9 = 0, l10 = 0, l11 = 0, l12 = 0, l13 = 0, 
      l14 = 0, l15 = 0, l16 = 0, l17 = 0, l18 = 0, l19 = 0, l20 = 0, l21 = 0, 
      l22 = 0, l23 = 0, l24 = 0;
  u64 l25 = 0, l26 = 0, l27 = 0;
  f64 l28 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5, i6, i7, 
      i8;
  u64 j0, j1, j2, j3, j4;
  f64 d0, d1, d2, d3, d4;
  i0 = g7;
  l21 = i0;
  i0 = g7;
  i1 = 560u;
  i0 += i1;
  g7 = i0;
  i0 = l21;
  i1 = 536u;
  i0 += i1;
  l10 = i0;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  d0 = p1;
  j0 = i64_reinterpret_f64(d0);
  l25 = j0;
  j1 = 0ull;
  i0 = (u64)((s64)j0 < (s64)j1);
  if (i0) {
    d0 = p1;
    d0 = -(d0);
    l28 = d0;
    p1 = d0;
    i0 = 2570u;
    l18 = i0;
    d0 = l28;
    j0 = i64_reinterpret_f64(d0);
    l25 = j0;
    i0 = 1u;
  } else {
    i0 = 2573u;
    i1 = 2576u;
    i2 = 2571u;
    i3 = p4;
    i4 = 1u;
    i3 &= i4;
    i1 = i3 ? i1 : i2;
    i2 = p4;
    i3 = 2048u;
    i2 &= i3;
    i0 = i2 ? i0 : i1;
    l18 = i0;
    i0 = p4;
    i1 = 2049u;
    i0 &= i1;
    i1 = 0u;
    i0 = i0 != i1;
  }
  l19 = i0;
  i0 = l21;
  i1 = 32u;
  i0 += i1;
  l7 = i0;
  i0 = l21;
  l13 = i0;
  l17 = i0;
  i0 = l13;
  i1 = 540u;
  i0 += i1;
  l12 = i0;
  i1 = 12u;
  i0 += i1;
  l16 = i0;
  j0 = l25;
  j1 = 9218868437227405312ull;
  j0 &= j1;
  j1 = 9218868437227405312ull;
  i0 = j0 == j1;
  if (i0) {
    i0 = p0;
    i1 = 32u;
    i2 = p2;
    i3 = l19;
    i4 = 3u;
    i3 += i4;
    p3 = i3;
    i4 = p4;
    i5 = 4294901759u;
    i4 &= i5;
    f48(i0, i1, i2, i3, i4);
    i0 = p0;
    i1 = l18;
    i2 = l19;
    f42(i0, i1, i2);
    i0 = p0;
    i1 = 2612u;
    i2 = 2597u;
    i3 = p5;
    i4 = 32u;
    i3 &= i4;
    i4 = 0u;
    i3 = i3 != i4;
    p5 = i3;
    i1 = i3 ? i1 : i2;
    i2 = 2589u;
    i3 = 2593u;
    i4 = p5;
    i2 = i4 ? i2 : i3;
    d3 = p1;
    d4 = p1;
    i3 = d3 != d4;
    i1 = i3 ? i1 : i2;
    i2 = 3u;
    f42(i0, i1, i2);
    i0 = p0;
    i1 = 32u;
    i2 = p2;
    i3 = p3;
    i4 = p4;
    i5 = 8192u;
    i4 ^= i5;
    f48(i0, i1, i2, i3, i4);
    i0 = p3;
  } else {
    d0 = p1;
    i1 = l10;
    d0 = f50(d0, i1);
    d1 = 2;
    d0 *= d1;
    p1 = d0;
    d1 = 0;
    i0 = d0 != d1;
    l6 = i0;
    if (i0) {
      i0 = l10;
      i1 = l10;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 4294967295u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    i0 = p5;
    i1 = 32u;
    i0 |= i1;
    l14 = i0;
    i1 = 97u;
    i0 = i0 == i1;
    if (i0) {
      i0 = l18;
      i1 = 9u;
      i0 += i1;
      i1 = l18;
      i2 = p5;
      i3 = 32u;
      i2 &= i3;
      l11 = i2;
      i0 = i2 ? i0 : i1;
      l8 = i0;
      i0 = 12u;
      i1 = p3;
      i0 -= i1;
      l7 = i0;
      i0 = !(i0);
      i1 = p3;
      i2 = 11u;
      i1 = i1 > i2;
      i0 |= i1;
      i0 = !(i0);
      if (i0) {
        d0 = 8;
        l28 = d0;
        L6: 
          d0 = l28;
          d1 = 16;
          d0 *= d1;
          l28 = d0;
          i0 = l7;
          i1 = 4294967295u;
          i0 += i1;
          l7 = i0;
          if (i0) {goto L6;}
        i0 = l8;
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
        i1 = 45u;
        i0 = i0 == i1;
        if (i0) {
          d0 = l28;
          d1 = p1;
          d1 = -(d1);
          d2 = l28;
          d1 -= d2;
          d0 += d1;
          d0 = -(d0);
        } else {
          d0 = p1;
          d1 = l28;
          d0 += d1;
          d1 = l28;
          d0 -= d1;
        }
        p1 = d0;
      }
      i0 = l16;
      i1 = 0u;
      i2 = l10;
      i2 = i32_load(Z_envZ_memory, (u64)(i2));
      l6 = i2;
      i1 -= i2;
      i2 = l6;
      i3 = l6;
      i4 = 0u;
      i3 = (u32)((s32)i3 < (s32)i4);
      i1 = i3 ? i1 : i2;
      j1 = (u64)(s64)(s32)(i1);
      i2 = l16;
      i1 = f47(j1, i2);
      l7 = i1;
      i0 = i0 == i1;
      if (i0) {
        i0 = l12;
        i1 = 11u;
        i0 += i1;
        l7 = i0;
        i1 = 48u;
        i32_store8(Z_envZ_memory, (u64)(i0), i1);
      }
      i0 = l19;
      i1 = 2u;
      i0 |= i1;
      l9 = i0;
      i0 = l7;
      i1 = 4294967295u;
      i0 += i1;
      i1 = l6;
      i2 = 31u;
      i1 = (u32)((s32)i1 >> (i2 & 31));
      i2 = 2u;
      i1 &= i2;
      i2 = 43u;
      i1 += i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = l7;
      i1 = 4294967294u;
      i0 += i1;
      l7 = i0;
      i1 = p5;
      i2 = 15u;
      i1 += i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = p3;
      i1 = 1u;
      i0 = (u32)((s32)i0 < (s32)i1);
      l12 = i0;
      i0 = p4;
      i1 = 8u;
      i0 &= i1;
      i0 = !(i0);
      l10 = i0;
      i0 = l13;
      p5 = i0;
      L9: 
        i0 = p5;
        i1 = l11;
        d2 = p1;
        i2 = I32_TRUNC_S_F64(d2);
        l6 = i2;
        i3 = 1968u;
        i2 += i3;
        i2 = i32_load8_u(Z_envZ_memory, (u64)(i2));
        i1 |= i2;
        i32_store8(Z_envZ_memory, (u64)(i0), i1);
        d0 = p1;
        i1 = l6;
        d1 = (f64)(s32)(i1);
        d0 -= d1;
        d1 = 16;
        d0 *= d1;
        p1 = d0;
        i0 = p5;
        i1 = 1u;
        i0 += i1;
        l6 = i0;
        i1 = l17;
        i0 -= i1;
        i1 = 1u;
        i0 = i0 == i1;
        if (i0) {
          i0 = l10;
          i1 = l12;
          d2 = p1;
          d3 = 0;
          i2 = d2 == d3;
          i1 &= i2;
          i0 &= i1;
          if (i0) {
            i0 = l6;
          } else {
            i0 = l6;
            i1 = 46u;
            i32_store8(Z_envZ_memory, (u64)(i0), i1);
            i0 = p5;
            i1 = 2u;
            i0 += i1;
          }
        } else {
          i0 = l6;
        }
        p5 = i0;
        d0 = p1;
        d1 = 0;
        i0 = d0 != d1;
        if (i0) {goto L9;}
      i0 = p3;
      i0 = !(i0);
      if (i0) {goto B13;}
      i0 = p5;
      i1 = 4294967294u;
      i2 = l17;
      i1 -= i2;
      i0 += i1;
      i1 = p3;
      i0 = (u32)((s32)i0 >= (s32)i1);
      if (i0) {goto B13;}
      i0 = l16;
      i1 = p3;
      i2 = 2u;
      i1 += i2;
      i0 += i1;
      i1 = l7;
      i0 -= i1;
      l12 = i0;
      i0 = l7;
      goto B12;
      B13:;
      i0 = p5;
      i1 = l16;
      i2 = l17;
      i1 -= i2;
      i2 = l7;
      i1 -= i2;
      i0 += i1;
      l12 = i0;
      i0 = l7;
      B12:;
      p3 = i0;
      i0 = p0;
      i1 = 32u;
      i2 = p2;
      i3 = l9;
      i4 = l12;
      i3 += i4;
      l6 = i3;
      i4 = p4;
      f48(i0, i1, i2, i3, i4);
      i0 = p0;
      i1 = l8;
      i2 = l9;
      f42(i0, i1, i2);
      i0 = p0;
      i1 = 48u;
      i2 = p2;
      i3 = l6;
      i4 = p4;
      i5 = 65536u;
      i4 ^= i5;
      f48(i0, i1, i2, i3, i4);
      i0 = p0;
      i1 = l13;
      i2 = p5;
      i3 = l17;
      i2 -= i3;
      p5 = i2;
      f42(i0, i1, i2);
      i0 = p0;
      i1 = 48u;
      i2 = l12;
      i3 = p5;
      i4 = l16;
      i5 = p3;
      i4 -= i5;
      p3 = i4;
      i3 += i4;
      i2 -= i3;
      i3 = 0u;
      i4 = 0u;
      f48(i0, i1, i2, i3, i4);
      i0 = p0;
      i1 = l7;
      i2 = p3;
      f42(i0, i1, i2);
      i0 = p0;
      i1 = 32u;
      i2 = p2;
      i3 = l6;
      i4 = p4;
      i5 = 8192u;
      i4 ^= i5;
      f48(i0, i1, i2, i3, i4);
      i0 = l6;
      goto B2;
    }
    i0 = l6;
    if (i0) {
      i0 = l10;
      i1 = l10;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 4294967268u;
      i1 += i2;
      l8 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      d0 = p1;
      d1 = 268435456;
      d0 *= d1;
      p1 = d0;
    } else {
      i0 = l10;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l8 = i0;
    }
    i0 = l7;
    i1 = l7;
    i2 = 288u;
    i1 += i2;
    i2 = l8;
    i3 = 0u;
    i2 = (u32)((s32)i2 < (s32)i3);
    i0 = i2 ? i0 : i1;
    l12 = i0;
    l6 = i0;
    L15: 
      i0 = l6;
      d1 = p1;
      i1 = I32_TRUNC_U_F64(d1);
      l7 = i1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l6;
      i1 = 4u;
      i0 += i1;
      l6 = i0;
      d0 = p1;
      i1 = l7;
      d1 = (f64)(i1);
      d0 -= d1;
      d1 = 1000000000;
      d0 *= d1;
      p1 = d0;
      d1 = 0;
      i0 = d0 != d1;
      if (i0) {goto L15;}
    i0 = l8;
    i1 = 0u;
    i0 = (u32)((s32)i0 > (s32)i1);
    if (i0) {
      i0 = l12;
      l7 = i0;
      L17: 
        i0 = l8;
        i1 = 29u;
        i2 = l8;
        i3 = 29u;
        i2 = (u32)((s32)i2 < (s32)i3);
        i0 = i2 ? i0 : i1;
        l11 = i0;
        i0 = l6;
        i1 = 4294967292u;
        i0 += i1;
        l8 = i0;
        i1 = l7;
        i0 = i0 >= i1;
        if (i0) {
          i0 = l11;
          j0 = (u64)(i0);
          l26 = j0;
          i0 = 0u;
          l9 = i0;
          L19: 
            i0 = l9;
            j0 = (u64)(i0);
            i1 = l8;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            j1 = (u64)(i1);
            j2 = l26;
            j1 <<= (j2 & 63);
            j0 += j1;
            l27 = j0;
            j1 = 1000000000ull;
            j0 = DIV_U(j0, j1);
            l25 = j0;
            i0 = l8;
            j1 = l27;
            j2 = l25;
            j3 = 1000000000ull;
            j2 *= j3;
            j1 -= j2;
            i64_store32(Z_envZ_memory, (u64)(i0), j1);
            j0 = l25;
            i0 = (u32)(j0);
            l9 = i0;
            i0 = l8;
            i1 = 4294967292u;
            i0 += i1;
            l8 = i0;
            i1 = l7;
            i0 = i0 >= i1;
            if (i0) {goto L19;}
          i0 = l9;
          if (i0) {
            i0 = l7;
            i1 = 4294967292u;
            i0 += i1;
            l7 = i0;
            i1 = l9;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
          }
        }
        i0 = l6;
        i1 = l7;
        i0 = i0 > i1;
        if (i0) {
          L23: 
            i0 = l6;
            i1 = 4294967292u;
            i0 += i1;
            l8 = i0;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
            if (i0) {goto B22;}
            i0 = l8;
            i1 = l7;
            i0 = i0 > i1;
            if (i0) {
              i0 = l8;
              l6 = i0;
              goto L23;
            } else {
              i0 = l8;
            }
          l6 = i0;
          B22:;
        }
        i0 = l10;
        i1 = l10;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i2 = l11;
        i1 -= i2;
        l8 = i1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l8;
        i1 = 0u;
        i0 = (u32)((s32)i0 > (s32)i1);
        if (i0) {goto L17;}
    } else {
      i0 = l12;
      l7 = i0;
    }
    i0 = 6u;
    i1 = p3;
    i2 = p3;
    i3 = 0u;
    i2 = (u32)((s32)i2 < (s32)i3);
    i0 = i2 ? i0 : i1;
    l11 = i0;
    i0 = l8;
    i1 = 0u;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {
      i0 = l11;
      i1 = 25u;
      i0 += i1;
      i1 = 9u;
      i0 = I32_DIV_S(i0, i1);
      i1 = 1u;
      i0 += i1;
      l15 = i0;
      i0 = l14;
      i1 = 102u;
      i0 = i0 == i1;
      l20 = i0;
      i0 = l6;
      p3 = i0;
      L26: 
        i0 = 0u;
        i1 = l8;
        i0 -= i1;
        l6 = i0;
        i1 = 9u;
        i2 = l6;
        i3 = 9u;
        i2 = (u32)((s32)i2 < (s32)i3);
        i0 = i2 ? i0 : i1;
        l9 = i0;
        i0 = l12;
        i1 = l7;
        i2 = p3;
        i1 = i1 < i2;
        if (i1) {
          i1 = 1u;
          i2 = l9;
          i1 <<= (i2 & 31);
          i2 = 4294967295u;
          i1 += i2;
          l22 = i1;
          i1 = 1000000000u;
          i2 = l9;
          i1 >>= (i2 & 31);
          l23 = i1;
          i1 = 0u;
          l8 = i1;
          i1 = l7;
          l6 = i1;
          L28: 
            i1 = l6;
            i2 = l8;
            i3 = l6;
            i3 = i32_load(Z_envZ_memory, (u64)(i3));
            l8 = i3;
            i4 = l9;
            i3 >>= (i4 & 31);
            i2 += i3;
            i32_store(Z_envZ_memory, (u64)(i1), i2);
            i1 = l23;
            i2 = l8;
            i3 = l22;
            i2 &= i3;
            i1 *= i2;
            l8 = i1;
            i1 = l6;
            i2 = 4u;
            i1 += i2;
            l6 = i1;
            i2 = p3;
            i1 = i1 < i2;
            if (i1) {goto L28;}
          i1 = l7;
          i2 = l7;
          i3 = 4u;
          i2 += i3;
          i3 = l7;
          i3 = i32_load(Z_envZ_memory, (u64)(i3));
          i1 = i3 ? i1 : i2;
          l7 = i1;
          i1 = l8;
          if (i1) {
            i1 = p3;
            i2 = l8;
            i32_store(Z_envZ_memory, (u64)(i1), i2);
            i1 = p3;
            i2 = 4u;
            i1 += i2;
            l6 = i1;
            i1 = l7;
          } else {
            i1 = p3;
            l6 = i1;
            i1 = l7;
          }
        } else {
          i1 = p3;
          l6 = i1;
          i1 = l7;
          i2 = l7;
          i3 = 4u;
          i2 += i3;
          i3 = l7;
          i3 = i32_load(Z_envZ_memory, (u64)(i3));
          i1 = i3 ? i1 : i2;
        }
        p3 = i1;
        i2 = l20;
        i0 = i2 ? i0 : i1;
        l7 = i0;
        i1 = l15;
        i2 = 2u;
        i1 <<= (i2 & 31);
        i0 += i1;
        i1 = l6;
        i2 = l6;
        i3 = l7;
        i2 -= i3;
        i3 = 2u;
        i2 = (u32)((s32)i2 >> (i3 & 31));
        i3 = l15;
        i2 = (u32)((s32)i2 > (s32)i3);
        i0 = i2 ? i0 : i1;
        l8 = i0;
        i0 = l10;
        i1 = l9;
        i2 = l10;
        i2 = i32_load(Z_envZ_memory, (u64)(i2));
        i1 += i2;
        l6 = i1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l6;
        i1 = 0u;
        i0 = (u32)((s32)i0 < (s32)i1);
        if (i0) {
          i0 = p3;
          l7 = i0;
          i0 = l8;
          p3 = i0;
          i0 = l6;
          l8 = i0;
          goto L26;
        }
    } else {
      i0 = l7;
      p3 = i0;
      i0 = l6;
      l8 = i0;
    }
    i0 = l12;
    l15 = i0;
    i0 = p3;
    i1 = l8;
    i0 = i0 < i1;
    if (i0) {
      i0 = l15;
      i1 = p3;
      i0 -= i1;
      i1 = 2u;
      i0 = (u32)((s32)i0 >> (i1 & 31));
      i1 = 9u;
      i0 *= i1;
      l7 = i0;
      i0 = p3;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l9 = i0;
      i1 = 10u;
      i0 = i0 >= i1;
      if (i0) {
        i0 = 10u;
        l6 = i0;
        L33: 
          i0 = l7;
          i1 = 1u;
          i0 += i1;
          l7 = i0;
          i0 = l9;
          i1 = l6;
          i2 = 10u;
          i1 *= i2;
          l6 = i1;
          i0 = i0 >= i1;
          if (i0) {goto L33;}
      }
    } else {
      i0 = 0u;
      l7 = i0;
    }
    i0 = l11;
    i1 = 0u;
    i2 = l7;
    i3 = l14;
    i4 = 102u;
    i3 = i3 == i4;
    i1 = i3 ? i1 : i2;
    i0 -= i1;
    i1 = l14;
    i2 = 103u;
    i1 = i1 == i2;
    l22 = i1;
    i2 = l11;
    i3 = 0u;
    i2 = i2 != i3;
    l23 = i2;
    i1 &= i2;
    i2 = 31u;
    i1 <<= (i2 & 31);
    i2 = 31u;
    i1 = (u32)((s32)i1 >> (i2 & 31));
    i0 += i1;
    l6 = i0;
    i1 = l8;
    i2 = l15;
    i1 -= i2;
    i2 = 2u;
    i1 = (u32)((s32)i1 >> (i2 & 31));
    i2 = 9u;
    i1 *= i2;
    i2 = 4294967287u;
    i1 += i2;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {
      i0 = l6;
      i1 = 9216u;
      i0 += i1;
      l6 = i0;
      i1 = 9u;
      i0 = I32_DIV_S(i0, i1);
      l14 = i0;
      i0 = l6;
      i1 = l14;
      i2 = 9u;
      i1 *= i2;
      i0 -= i1;
      l6 = i0;
      i1 = 8u;
      i0 = (u32)((s32)i0 < (s32)i1);
      if (i0) {
        i0 = 10u;
        l9 = i0;
        L36: 
          i0 = l6;
          i1 = 1u;
          i0 += i1;
          l10 = i0;
          i0 = l9;
          i1 = 10u;
          i0 *= i1;
          l9 = i0;
          i0 = l6;
          i1 = 7u;
          i0 = (u32)((s32)i0 < (s32)i1);
          if (i0) {
            i0 = l10;
            l6 = i0;
            goto L36;
          }
      } else {
        i0 = 10u;
        l9 = i0;
      }
      i0 = l14;
      i1 = 2u;
      i0 <<= (i1 & 31);
      i1 = l12;
      i0 += i1;
      i1 = 4294963204u;
      i0 += i1;
      l6 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l14 = i0;
      i1 = l9;
      i0 = DIV_U(i0, i1);
      l20 = i0;
      i0 = l8;
      i1 = l6;
      i2 = 4u;
      i1 += i2;
      i0 = i0 == i1;
      l24 = i0;
      i1 = l14;
      i2 = l9;
      i3 = l20;
      i2 *= i3;
      i1 -= i2;
      l10 = i1;
      i1 = !(i1);
      i0 &= i1;
      i0 = !(i0);
      if (i0) {
        d0 = 9007199254740994;
        d1 = 9007199254740992;
        i2 = l20;
        i3 = 1u;
        i2 &= i3;
        d0 = i2 ? d0 : d1;
        p1 = d0;
        d0 = 0.5;
        d1 = 1;
        d2 = 1.5;
        i3 = l24;
        i4 = l10;
        i5 = l9;
        i6 = 1u;
        i5 >>= (i6 & 31);
        l20 = i5;
        i4 = i4 == i5;
        i3 &= i4;
        d1 = i3 ? d1 : d2;
        i2 = l10;
        i3 = l20;
        i2 = i2 < i3;
        d0 = i2 ? d0 : d1;
        l28 = d0;
        i0 = l19;
        if (i0) {
          d0 = l28;
          d0 = -(d0);
          d1 = l28;
          i2 = l18;
          i2 = i32_load8_s(Z_envZ_memory, (u64)(i2));
          i3 = 45u;
          i2 = i2 == i3;
          l20 = i2;
          d0 = i2 ? d0 : d1;
          l28 = d0;
          d0 = p1;
          d0 = -(d0);
          d1 = p1;
          i2 = l20;
          d0 = i2 ? d0 : d1;
          p1 = d0;
        }
        i0 = l6;
        i1 = l14;
        i2 = l10;
        i1 -= i2;
        l10 = i1;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        d0 = p1;
        d1 = l28;
        d0 += d1;
        d1 = p1;
        i0 = d0 != d1;
        if (i0) {
          i0 = l6;
          i1 = l9;
          i2 = l10;
          i1 += i2;
          l7 = i1;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l7;
          i1 = 999999999u;
          i0 = i0 > i1;
          if (i0) {
            L42: 
              i0 = l6;
              i1 = 0u;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
              i0 = l6;
              i1 = 4294967292u;
              i0 += i1;
              l6 = i0;
              i1 = p3;
              i0 = i0 < i1;
              if (i0) {
                i0 = p3;
                i1 = 4294967292u;
                i0 += i1;
                p3 = i0;
                i1 = 0u;
                i32_store(Z_envZ_memory, (u64)(i0), i1);
              }
              i0 = l6;
              i1 = l6;
              i1 = i32_load(Z_envZ_memory, (u64)(i1));
              i2 = 1u;
              i1 += i2;
              l7 = i1;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
              i0 = l7;
              i1 = 999999999u;
              i0 = i0 > i1;
              if (i0) {goto L42;}
          }
          i0 = l15;
          i1 = p3;
          i0 -= i1;
          i1 = 2u;
          i0 = (u32)((s32)i0 >> (i1 & 31));
          i1 = 9u;
          i0 *= i1;
          l7 = i0;
          i0 = p3;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l10 = i0;
          i1 = 10u;
          i0 = i0 >= i1;
          if (i0) {
            i0 = 10u;
            l9 = i0;
            L45: 
              i0 = l7;
              i1 = 1u;
              i0 += i1;
              l7 = i0;
              i0 = l10;
              i1 = l9;
              i2 = 10u;
              i1 *= i2;
              l9 = i1;
              i0 = i0 >= i1;
              if (i0) {goto L45;}
          }
        }
      }
      i0 = l7;
      l9 = i0;
      i0 = l6;
      i1 = 4u;
      i0 += i1;
      l7 = i0;
      i1 = l8;
      i2 = l8;
      i3 = l7;
      i2 = i2 > i3;
      i0 = i2 ? i0 : i1;
      l6 = i0;
      i0 = p3;
    } else {
      i0 = l7;
      l9 = i0;
      i0 = l8;
      l6 = i0;
      i0 = p3;
    }
    l7 = i0;
    i0 = l6;
    i1 = l7;
    i0 = i0 > i1;
    if (i0) {
      i0 = l6;
      p3 = i0;
      L48: 
        i0 = p3;
        i1 = 4294967292u;
        i0 += i1;
        l6 = i0;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        if (i0) {
          i0 = p3;
          l6 = i0;
          i0 = 1u;
          goto B47;
        }
        i0 = l6;
        i1 = l7;
        i0 = i0 > i1;
        if (i0) {
          i0 = l6;
          p3 = i0;
          goto L48;
        } else {
          i0 = 0u;
        }
      B47:;
    } else {
      i0 = 0u;
    }
    l14 = i0;
    i0 = l22;
    if (i0) {
      i0 = l23;
      i1 = 1u;
      i0 ^= i1;
      i1 = 1u;
      i0 &= i1;
      i1 = l11;
      i0 += i1;
      p3 = i0;
      i1 = l9;
      i0 = (u32)((s32)i0 > (s32)i1);
      i1 = l9;
      i2 = 4294967291u;
      i1 = (u32)((s32)i1 > (s32)i2);
      i0 &= i1;
      if (i0) {
        i0 = p3;
        i1 = 4294967295u;
        i0 += i1;
        i1 = l9;
        i0 -= i1;
        l10 = i0;
        i0 = p5;
        i1 = 4294967295u;
        i0 += i1;
      } else {
        i0 = p3;
        i1 = 4294967295u;
        i0 += i1;
        l10 = i0;
        i0 = p5;
        i1 = 4294967294u;
        i0 += i1;
      }
      p5 = i0;
      i0 = p4;
      i1 = 8u;
      i0 &= i1;
      if (i0) {
        i0 = l10;
      } else {
        i0 = l14;
        if (i0) {
          i0 = l6;
          i1 = 4294967292u;
          i0 += i1;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l11 = i0;
          if (i0) {
            i0 = l11;
            i1 = 10u;
            i0 = REM_U(i0, i1);
            if (i0) {
              i0 = 0u;
              p3 = i0;
            } else {
              i0 = 0u;
              p3 = i0;
              i0 = 10u;
              l8 = i0;
              L57: 
                i0 = p3;
                i1 = 1u;
                i0 += i1;
                p3 = i0;
                i0 = l11;
                i1 = l8;
                i2 = 10u;
                i1 *= i2;
                l8 = i1;
                i0 = REM_U(i0, i1);
                i0 = !(i0);
                if (i0) {goto L57;}
            }
          } else {
            i0 = 9u;
            p3 = i0;
          }
        } else {
          i0 = 9u;
          p3 = i0;
        }
        i0 = l6;
        i1 = l15;
        i0 -= i1;
        i1 = 2u;
        i0 = (u32)((s32)i0 >> (i1 & 31));
        i1 = 9u;
        i0 *= i1;
        i1 = 4294967287u;
        i0 += i1;
        l8 = i0;
        i0 = p5;
        i1 = 32u;
        i0 |= i1;
        i1 = 102u;
        i0 = i0 == i1;
        if (i0) {
          i0 = l10;
          i1 = l8;
          i2 = p3;
          i1 -= i2;
          p3 = i1;
          i2 = 0u;
          i3 = p3;
          i4 = 0u;
          i3 = (u32)((s32)i3 > (s32)i4);
          i1 = i3 ? i1 : i2;
          p3 = i1;
          i2 = l10;
          i3 = p3;
          i2 = (u32)((s32)i2 < (s32)i3);
          i0 = i2 ? i0 : i1;
        } else {
          i0 = l10;
          i1 = l8;
          i2 = l9;
          i1 += i2;
          i2 = p3;
          i1 -= i2;
          p3 = i1;
          i2 = 0u;
          i3 = p3;
          i4 = 0u;
          i3 = (u32)((s32)i3 > (s32)i4);
          i1 = i3 ? i1 : i2;
          p3 = i1;
          i2 = l10;
          i3 = p3;
          i2 = (u32)((s32)i2 < (s32)i3);
          i0 = i2 ? i0 : i1;
        }
      }
    } else {
      i0 = l11;
    }
    p3 = i0;
    i0 = 0u;
    i1 = l9;
    i0 -= i1;
    l8 = i0;
    i0 = p0;
    i1 = 32u;
    i2 = p2;
    i3 = p5;
    i4 = 32u;
    i3 |= i4;
    i4 = 102u;
    i3 = i3 == i4;
    l11 = i3;
    if (i3) {
      i3 = 0u;
      l8 = i3;
      i3 = l9;
      i4 = 0u;
      i5 = l9;
      i6 = 0u;
      i5 = (u32)((s32)i5 > (s32)i6);
      i3 = i5 ? i3 : i4;
    } else {
      i3 = l16;
      l10 = i3;
      i4 = l8;
      i5 = l9;
      i6 = l9;
      i7 = 0u;
      i6 = (u32)((s32)i6 < (s32)i7);
      i4 = i6 ? i4 : i5;
      j4 = (u64)(s64)(s32)(i4);
      i5 = l10;
      i4 = f47(j4, i5);
      l8 = i4;
      i3 -= i4;
      i4 = 2u;
      i3 = (u32)((s32)i3 < (s32)i4);
      if (i3) {
        L61: 
          i3 = l8;
          i4 = 4294967295u;
          i3 += i4;
          l8 = i3;
          i4 = 48u;
          i32_store8(Z_envZ_memory, (u64)(i3), i4);
          i3 = l10;
          i4 = l8;
          i3 -= i4;
          i4 = 2u;
          i3 = (u32)((s32)i3 < (s32)i4);
          if (i3) {goto L61;}
      }
      i3 = l8;
      i4 = 4294967295u;
      i3 += i4;
      i4 = l9;
      i5 = 31u;
      i4 = (u32)((s32)i4 >> (i5 & 31));
      i5 = 2u;
      i4 &= i5;
      i5 = 43u;
      i4 += i5;
      i32_store8(Z_envZ_memory, (u64)(i3), i4);
      i3 = l8;
      i4 = 4294967294u;
      i3 += i4;
      l8 = i3;
      i4 = p5;
      i32_store8(Z_envZ_memory, (u64)(i3), i4);
      i3 = l10;
      i4 = l8;
      i3 -= i4;
    }
    i4 = p3;
    i5 = l19;
    i6 = 1u;
    i5 += i6;
    i4 += i5;
    i5 = 1u;
    i6 = p4;
    i7 = 3u;
    i6 >>= (i7 & 31);
    i7 = 1u;
    i6 &= i7;
    i7 = p3;
    i8 = 0u;
    i7 = i7 != i8;
    l10 = i7;
    i5 = i7 ? i5 : i6;
    i4 += i5;
    i3 += i4;
    l9 = i3;
    i4 = p4;
    f48(i0, i1, i2, i3, i4);
    i0 = p0;
    i1 = l18;
    i2 = l19;
    f42(i0, i1, i2);
    i0 = p0;
    i1 = 48u;
    i2 = p2;
    i3 = l9;
    i4 = p4;
    i5 = 65536u;
    i4 ^= i5;
    f48(i0, i1, i2, i3, i4);
    i0 = l11;
    if (i0) {
      i0 = l13;
      i1 = 9u;
      i0 += i1;
      l8 = i0;
      l11 = i0;
      i0 = l13;
      i1 = 8u;
      i0 += i1;
      l16 = i0;
      i0 = l12;
      i1 = l7;
      i2 = l7;
      i3 = l12;
      i2 = i2 > i3;
      i0 = i2 ? i0 : i1;
      l15 = i0;
      l7 = i0;
      L63: 
        i0 = l7;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        j0 = (u64)(i0);
        i1 = l8;
        i0 = f47(j0, i1);
        p5 = i0;
        i0 = l7;
        i1 = l15;
        i0 = i0 == i1;
        if (i0) {
          i0 = p5;
          i1 = l8;
          i0 = i0 == i1;
          if (i0) {
            i0 = l16;
            i1 = 48u;
            i32_store8(Z_envZ_memory, (u64)(i0), i1);
            i0 = l16;
            p5 = i0;
          }
        } else {
          i0 = p5;
          i1 = l13;
          i0 = i0 > i1;
          if (i0) {
            i0 = l13;
            i1 = 48u;
            i2 = p5;
            i3 = l17;
            i2 -= i3;
            i0 = _memset(i0, i1, i2);
            L67: 
              i0 = p5;
              i1 = 4294967295u;
              i0 += i1;
              p5 = i0;
              i1 = l13;
              i0 = i0 > i1;
              if (i0) {goto L67;}
          }
        }
        i0 = p0;
        i1 = p5;
        i2 = l11;
        i3 = p5;
        i2 -= i3;
        f42(i0, i1, i2);
        i0 = l7;
        i1 = 4u;
        i0 += i1;
        p5 = i0;
        i1 = l12;
        i0 = i0 <= i1;
        if (i0) {
          i0 = p5;
          l7 = i0;
          goto L63;
        }
      i0 = p4;
      i1 = 8u;
      i0 &= i1;
      i0 = !(i0);
      i1 = l10;
      i2 = 1u;
      i1 ^= i2;
      i0 &= i1;
      i0 = !(i0);
      if (i0) {
        i0 = p0;
        i1 = 2601u;
        i2 = 1u;
        f42(i0, i1, i2);
      }
      i0 = p0;
      i1 = 48u;
      i2 = p5;
      i3 = l6;
      i2 = i2 < i3;
      i3 = p3;
      i4 = 0u;
      i3 = (u32)((s32)i3 > (s32)i4);
      i2 &= i3;
      if (i2) {
        L71: 
          i2 = p5;
          i2 = i32_load(Z_envZ_memory, (u64)(i2));
          j2 = (u64)(i2);
          i3 = l8;
          i2 = f47(j2, i3);
          l7 = i2;
          i3 = l13;
          i2 = i2 > i3;
          if (i2) {
            i2 = l13;
            i3 = 48u;
            i4 = l7;
            i5 = l17;
            i4 -= i5;
            i2 = _memset(i2, i3, i4);
            L73: 
              i2 = l7;
              i3 = 4294967295u;
              i2 += i3;
              l7 = i2;
              i3 = l13;
              i2 = i2 > i3;
              if (i2) {goto L73;}
          }
          i2 = p0;
          i3 = l7;
          i4 = p3;
          i5 = 9u;
          i6 = p3;
          i7 = 9u;
          i6 = (u32)((s32)i6 < (s32)i7);
          i4 = i6 ? i4 : i5;
          f42(i2, i3, i4);
          i2 = p3;
          i3 = 4294967287u;
          i2 += i3;
          l7 = i2;
          i2 = p5;
          i3 = 4u;
          i2 += i3;
          p5 = i2;
          i3 = l6;
          i2 = i2 < i3;
          i3 = p3;
          i4 = 9u;
          i3 = (u32)((s32)i3 > (s32)i4);
          i2 &= i3;
          if (i2) {
            i2 = l7;
            p3 = i2;
            goto L71;
          } else {
            i2 = l7;
          }
      } else {
        i2 = p3;
      }
      i3 = 9u;
      i2 += i3;
      i3 = 9u;
      i4 = 0u;
      f48(i0, i1, i2, i3, i4);
    } else {
      i0 = p0;
      i1 = 48u;
      i2 = l7;
      i3 = l6;
      i4 = l7;
      i5 = 4u;
      i4 += i5;
      i5 = l14;
      i3 = i5 ? i3 : i4;
      l15 = i3;
      i2 = i2 < i3;
      i3 = p3;
      i4 = 4294967295u;
      i3 = (u32)((s32)i3 > (s32)i4);
      i2 &= i3;
      if (i2) {
        i2 = p4;
        i3 = 8u;
        i2 &= i3;
        i2 = !(i2);
        l19 = i2;
        i2 = l13;
        i3 = 9u;
        i2 += i3;
        l11 = i2;
        l18 = i2;
        i2 = 0u;
        i3 = l17;
        i2 -= i3;
        l17 = i2;
        i2 = l13;
        i3 = 8u;
        i2 += i3;
        l10 = i2;
        i2 = p3;
        p5 = i2;
        i2 = l7;
        l6 = i2;
        L76: 
          i2 = l11;
          i3 = l6;
          i3 = i32_load(Z_envZ_memory, (u64)(i3));
          j3 = (u64)(i3);
          i4 = l11;
          i3 = f47(j3, i4);
          p3 = i3;
          i2 = i2 == i3;
          if (i2) {
            i2 = l10;
            i3 = 48u;
            i32_store8(Z_envZ_memory, (u64)(i2), i3);
            i2 = l10;
            p3 = i2;
          }
          i2 = l6;
          i3 = l7;
          i2 = i2 == i3;
          if (i2) {
            i2 = p3;
            i3 = 1u;
            i2 += i3;
            l12 = i2;
            i2 = p0;
            i3 = p3;
            i4 = 1u;
            f42(i2, i3, i4);
            i2 = l19;
            i3 = p5;
            i4 = 1u;
            i3 = (u32)((s32)i3 < (s32)i4);
            i2 &= i3;
            if (i2) {
              i2 = l12;
              p3 = i2;
              goto B78;
            }
            i2 = p0;
            i3 = 2601u;
            i4 = 1u;
            f42(i2, i3, i4);
            i2 = l12;
            p3 = i2;
          } else {
            i2 = p3;
            i3 = l13;
            i2 = i2 <= i3;
            if (i2) {goto B78;}
            i2 = l13;
            i3 = 48u;
            i4 = p3;
            i5 = l17;
            i4 += i5;
            i2 = _memset(i2, i3, i4);
            L81: 
              i2 = p3;
              i3 = 4294967295u;
              i2 += i3;
              p3 = i2;
              i3 = l13;
              i2 = i2 > i3;
              if (i2) {goto L81;}
          }
          B78:;
          i2 = p0;
          i3 = p3;
          i4 = l18;
          i5 = p3;
          i4 -= i5;
          p3 = i4;
          i5 = p5;
          i6 = p5;
          i7 = p3;
          i6 = (u32)((s32)i6 > (s32)i7);
          i4 = i6 ? i4 : i5;
          f42(i2, i3, i4);
          i2 = l6;
          i3 = 4u;
          i2 += i3;
          l6 = i2;
          i3 = l15;
          i2 = i2 < i3;
          i3 = p5;
          i4 = p3;
          i3 -= i4;
          p5 = i3;
          i4 = 4294967295u;
          i3 = (u32)((s32)i3 > (s32)i4);
          i2 &= i3;
          if (i2) {goto L76;}
          i2 = p5;
      } else {
        i2 = p3;
      }
      i3 = 18u;
      i2 += i3;
      i3 = 18u;
      i4 = 0u;
      f48(i0, i1, i2, i3, i4);
      i0 = p0;
      i1 = l8;
      i2 = l16;
      i3 = l8;
      i2 -= i3;
      f42(i0, i1, i2);
    }
    i0 = p0;
    i1 = 32u;
    i2 = p2;
    i3 = l9;
    i4 = p4;
    i5 = 8192u;
    i4 ^= i5;
    f48(i0, i1, i2, i3, i4);
    i0 = l9;
    B2:;
  }
  p0 = i0;
  i0 = l21;
  g7 = i0;
  i0 = p2;
  i1 = p0;
  i2 = p0;
  i3 = p2;
  i2 = (u32)((s32)i2 < (s32)i3);
  i0 = i2 ? i0 : i1;
  FUNC_EPILOGUE;
  return i0;
}

static f64 f50(f64 p0, u32 p1) {
  u32 l2 = 0;
  u64 l3 = 0, l4 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  u64 j0, j1;
  f64 d0, d1, d2;
  d0 = p0;
  j0 = i64_reinterpret_f64(d0);
  l3 = j0;
  j1 = 52ull;
  j0 >>= (j1 & 63);
  l4 = j0;
  i0 = (u32)(j0);
  i1 = 2047u;
  i0 &= i1;
  l2 = i0;
  if (i0) {
    i0 = l2;
    i1 = 2047u;
    i0 = i0 == i1;
    if (i0) {
      goto B0;
    } else {
      goto B1;
    }
    UNREACHABLE;
  }
  i0 = p1;
  d1 = p0;
  d2 = 0;
  i1 = d1 != d2;
  if (i1) {
    d1 = p0;
    d2 = 1.8446744073709552e+19;
    d1 *= d2;
    i2 = p1;
    d1 = f50(d1, i2);
    p0 = d1;
    i1 = p1;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 4294967232u;
    i1 += i2;
  } else {
    i1 = 0u;
  }
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  goto B0;
  B1:;
  i0 = p1;
  j1 = l4;
  i1 = (u32)(j1);
  i2 = 2047u;
  i1 &= i2;
  i2 = 4294966274u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  j0 = l3;
  j1 = 9227875636482146303ull;
  j0 &= j1;
  j1 = 4602678819172646912ull;
  j0 |= j1;
  d0 = f64_reinterpret_i64(j0);
  p0 = d0;
  B0:;
  d0 = p0;
  FUNC_EPILOGUE;
  return d0;
}

static void f51(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0, l5 = 0, l6 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  i0 = p2;
  i1 = 16u;
  i0 += i1;
  l4 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l3 = i0;
  if (i0) {goto B1;}
  i0 = p2;
  i0 = f52(i0);
  i0 = !(i0);
  if (i0) {
    i0 = l4;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    goto B1;
  }
  goto B0;
  B1:;
  i0 = l3;
  i1 = p2;
  i2 = 20u;
  i1 += i2;
  l5 = i1;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  l4 = i1;
  i0 -= i1;
  i1 = p1;
  i0 = i0 < i1;
  if (i0) {
    i0 = p2;
    i1 = p0;
    i2 = p1;
    i3 = p2;
    i3 = i32_load(Z_envZ_memory, (u64)(i3 + 36));
    i4 = 7u;
    i3 &= i4;
    i4 = 2u;
    i3 += i4;
    i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, u32, u32), 0, i3, i0, i1, i2);
    goto B0;
  }
  i0 = p1;
  i0 = !(i0);
  i1 = p2;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 75));
  i2 = 0u;
  i1 = (u32)((s32)i1 < (s32)i2);
  i0 |= i1;
  if (i0) {
    i0 = 0u;
  } else {
    i0 = p1;
    l3 = i0;
    L6: 
      i0 = p0;
      i1 = l3;
      i2 = 4294967295u;
      i1 += i2;
      l6 = i1;
      i0 += i1;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      i1 = 10u;
      i0 = i0 != i1;
      if (i0) {
        i0 = l6;
        if (i0) {
          i0 = l6;
          l3 = i0;
          goto L6;
        } else {
          i0 = 0u;
          goto B5;
        }
        UNREACHABLE;
      }
    i0 = p2;
    i1 = p0;
    i2 = l3;
    i3 = p2;
    i3 = i32_load(Z_envZ_memory, (u64)(i3 + 36));
    i4 = 7u;
    i3 &= i4;
    i4 = 2u;
    i3 += i4;
    i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, u32, u32), 0, i3, i0, i1, i2);
    i1 = l3;
    i0 = i0 < i1;
    if (i0) {goto B0;}
    i0 = p0;
    i1 = l3;
    i0 += i1;
    p0 = i0;
    i0 = p1;
    i1 = l3;
    i0 -= i1;
    p1 = i0;
    i0 = l5;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l4 = i0;
    i0 = 0u;
    B5:;
  }
  i0 = l4;
  i1 = p0;
  i2 = p1;
  i0 = _memcpy(i0, i1, i2);
  i0 = l5;
  i1 = p1;
  i2 = l5;
  i2 = i32_load(Z_envZ_memory, (u64)(i2));
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  B0:;
  FUNC_EPILOGUE;
}

static u32 f52(u32 p0) {
  u32 l1 = 0, l2 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  i0 = p0;
  i1 = 74u;
  i0 += i1;
  l2 = i0;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i0 = l2;
  i1 = l1;
  i2 = l1;
  i3 = 255u;
  i2 += i3;
  i1 |= i2;
  i32_store8(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i1 = 8u;
  i0 &= i1;
  if (i0) {
    i0 = p0;
    i1 = l1;
    i2 = 32u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4294967295u;
  } else {
    i0 = p0;
    i1 = 8u;
    i0 += i1;
    i1 = 0u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 4u;
    i0 += i1;
    i1 = 0u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 28u;
    i0 += i1;
    i1 = p0;
    i2 = 44u;
    i1 += i2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    l1 = i1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 20u;
    i0 += i1;
    i1 = l1;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p0;
    i1 = 16u;
    i0 += i1;
    i1 = l1;
    i2 = p0;
    i3 = 48u;
    i2 += i3;
    i2 = i32_load(Z_envZ_memory, (u64)(i2));
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 0u;
  }
  FUNC_EPILOGUE;
  return i0;
}

static void f53(u32 p0) {
  u32 l1 = 0, l2 = 0, l3 = 0, l4 = 0, l5 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  i0 = g7;
  l2 = i0;
  i0 = g7;
  i1 = 16u;
  i0 += i1;
  g7 = i0;
  i0 = l2;
  l3 = i0;
  i1 = 10u;
  i32_store8(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 16u;
  i0 += i1;
  l1 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l4 = i0;
  if (i0) {goto B1;}
  i0 = p0;
  i0 = f52(i0);
  i0 = !(i0);
  if (i0) {
    i0 = l1;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l4 = i0;
    goto B1;
  }
  goto B0;
  B1:;
  i0 = p0;
  i1 = 20u;
  i0 += i1;
  l1 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l5 = i0;
  i1 = l4;
  i0 = i0 < i1;
  if (i0) {
    i0 = p0;
    i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 75));
    i1 = 10u;
    i0 = i0 != i1;
    if (i0) {
      i0 = l1;
      i1 = l5;
      i2 = 1u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l5;
      i1 = 10u;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      goto B0;
    }
  }
  i0 = p0;
  i1 = l3;
  i2 = 1u;
  i3 = p0;
  i3 = i32_load(Z_envZ_memory, (u64)(i3 + 36));
  i4 = 7u;
  i3 &= i4;
  i4 = 2u;
  i3 += i4;
  i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, u32, u32), 0, i3, i0, i1, i2);
  i1 = 1u;
  i0 = i0 == i1;
  if (i0) {
    i0 = l3;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
  } else {
    i0 = 4294967295u;
  }
  B0:;
  i0 = l2;
  g7 = i0;
  FUNC_EPILOGUE;
}

static void f54(u32 p0, u32 p1) {
  u32 l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0, l9 = 0, 
      l10 = 0, l11 = 0, l12 = 0, l13 = 0, l14 = 0, l15 = 0, l16 = 0, l17 = 0, 
      l18 = 0, l19 = 0, l20 = 0, l21 = 0, l22 = 0, l23 = 0, l24 = 0, l25 = 0, 
      l26 = 0, l27 = 0, l28 = 0, l29 = 0;
  u64 l30 = 0;
  f64 l31 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  u64 j0, j1, j2;
  f32 f1;
  f64 d0, d1;
  i0 = 2541u;
  l2 = i0;
  i0 = g7;
  l20 = i0;
  i0 = g7;
  i1 = 288u;
  i0 += i1;
  g7 = i0;
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76));
  i1 = 4294967295u;
  i0 = (u32)((s32)i0 > (s32)i1);
  if (i0) {
    i0 = 1u;
  } else {
    i0 = 0u;
  }
  i0 = l20;
  i1 = 264u;
  i0 += i1;
  l19 = i0;
  i0 = l20;
  l12 = i0;
  i1 = 260u;
  i0 += i1;
  l22 = i0;
  i0 = l12;
  i1 = 272u;
  i0 += i1;
  l23 = i0;
  i0 = 2541u;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
  l7 = i0;
  if (i0) {
    i0 = p0;
    i1 = 4u;
    i0 += i1;
    l5 = i0;
    i0 = p0;
    i1 = 100u;
    i0 += i1;
    l13 = i0;
    i0 = p0;
    i1 = 108u;
    i0 += i1;
    l17 = i0;
    i0 = p0;
    i1 = 8u;
    i0 += i1;
    l18 = i0;
    i0 = l12;
    i1 = 10u;
    i0 += i1;
    l24 = i0;
    i0 = l12;
    i1 = 33u;
    i0 += i1;
    l26 = i0;
    i0 = l12;
    i1 = 46u;
    i0 += i1;
    l27 = i0;
    i0 = l12;
    i1 = 94u;
    i0 += i1;
    l28 = i0;
    i0 = l19;
    i1 = 4u;
    i0 += i1;
    l29 = i0;
    L6: 
      i0 = l7;
      i1 = 255u;
      i0 &= i1;
      i0 = f33(i0);
      if (i0) {
        L9: 
          i0 = l2;
          i1 = 1u;
          i0 += i1;
          l7 = i0;
          i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
          i0 = f33(i0);
          if (i0) {
            i0 = l7;
            l2 = i0;
            goto L9;
          }
        i0 = p0;
        i1 = 0u;
        f30(i0, i1);
        L11: 
          i0 = l5;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l7 = i0;
          i1 = l13;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i0 = i0 < i1;
          if (i0) {
            i0 = l5;
            i1 = l7;
            i2 = 1u;
            i1 += i2;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            i0 = l7;
            i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
          } else {
            i0 = p0;
            i0 = f32_0(i0);
          }
          i0 = f33(i0);
          if (i0) {goto L11;}
        i0 = l13;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        if (i0) {
          i0 = l5;
          i1 = l5;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i2 = 4294967295u;
          i1 += i2;
          l7 = i1;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
        } else {
          i0 = l5;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l7 = i0;
        }
        i0 = l3;
        i1 = l17;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i0 += i1;
        i1 = l7;
        i0 += i1;
        i1 = l18;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i0 -= i1;
        l3 = i0;
      } else {
        i0 = l2;
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
        i1 = 37u;
        i0 = i0 == i1;
        l9 = i0;
        if (i0) {
          i0 = l2;
          i1 = 1u;
          i0 += i1;
          l7 = i0;
          i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
          l14 = i0;
          i1 = 37u;
          i0 -= i1;
          switch (i0) {
            case 0: goto B16;
            case 1: goto B18;
            case 2: goto B18;
            case 3: goto B18;
            case 4: goto B18;
            case 5: goto B19;
            default: goto B18;
          }
          B19:;
          i0 = 0u;
          l9 = i0;
          i0 = l2;
          i1 = 2u;
          i0 += i1;
          goto B17;
          B18:;
          i0 = l14;
          i1 = 255u;
          i0 &= i1;
          i0 = f29(i0);
          if (i0) {
            i0 = l2;
            i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 2));
            i1 = 36u;
            i0 = i0 == i1;
            if (i0) {
              i0 = p1;
              i1 = l7;
              i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
              i2 = 4294967248u;
              i1 += i2;
              i0 = f55(i0, i1);
              l9 = i0;
              i0 = l2;
              i1 = 3u;
              i0 += i1;
              goto B17;
            }
          }
          i0 = p1;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          i1 = 3u;
          i0 += i1;
          i1 = 4294967292u;
          i0 &= i1;
          l2 = i0;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l9 = i0;
          i0 = p1;
          i1 = l2;
          i2 = 4u;
          i1 += i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l7;
          B17:;
          l2 = i0;
          i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
          i0 = f29(i0);
          if (i0) {
            i0 = 0u;
            l14 = i0;
            L23: 
              i0 = l2;
              i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
              i1 = l14;
              i2 = 10u;
              i1 *= i2;
              i2 = 4294967248u;
              i1 += i2;
              i0 += i1;
              l14 = i0;
              i0 = l2;
              i1 = 1u;
              i0 += i1;
              l2 = i0;
              i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
              i0 = f29(i0);
              if (i0) {goto L23;}
          } else {
            i0 = 0u;
            l14 = i0;
          }
          i0 = l2;
          i1 = 1u;
          i0 += i1;
          l11 = i0;
          i0 = l2;
          i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
          l8 = i0;
          i1 = 109u;
          i0 = i0 == i1;
          if (i0) {
            i0 = 0u;
            l6 = i0;
            i0 = l2;
            i1 = 2u;
            i0 += i1;
            l2 = i0;
            i0 = l11;
            l4 = i0;
            i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
            l11 = i0;
            i0 = 0u;
            l10 = i0;
            i0 = l9;
            i1 = 0u;
            i0 = i0 != i1;
          } else {
            i0 = l2;
            l4 = i0;
            i0 = l11;
            l2 = i0;
            i0 = l8;
            l11 = i0;
            i0 = 0u;
          }
          l7 = i0;
          i0 = l11;
          i1 = 24u;
          i0 <<= (i1 & 31);
          i1 = 24u;
          i0 = (u32)((s32)i0 >> (i1 & 31));
          i1 = 65u;
          i0 -= i1;
          switch (i0) {
            case 0: goto B26;
            case 1: goto B4;
            case 2: goto B26;
            case 3: goto B4;
            case 4: goto B26;
            case 5: goto B26;
            case 6: goto B26;
            case 7: goto B4;
            case 8: goto B4;
            case 9: goto B4;
            case 10: goto B4;
            case 11: goto B27;
            case 12: goto B4;
            case 13: goto B4;
            case 14: goto B4;
            case 15: goto B4;
            case 16: goto B4;
            case 17: goto B4;
            case 18: goto B26;
            case 19: goto B4;
            case 20: goto B4;
            case 21: goto B4;
            case 22: goto B4;
            case 23: goto B26;
            case 24: goto B4;
            case 25: goto B4;
            case 26: goto B26;
            case 27: goto B4;
            case 28: goto B4;
            case 29: goto B4;
            case 30: goto B4;
            case 31: goto B4;
            case 32: goto B26;
            case 33: goto B4;
            case 34: goto B26;
            case 35: goto B26;
            case 36: goto B26;
            case 37: goto B26;
            case 38: goto B26;
            case 39: goto B31;
            case 40: goto B26;
            case 41: goto B29;
            case 42: goto B4;
            case 43: goto B30;
            case 44: goto B4;
            case 45: goto B26;
            case 46: goto B26;
            case 47: goto B26;
            case 48: goto B4;
            case 49: goto B4;
            case 50: goto B26;
            case 51: goto B28;
            case 52: goto B26;
            case 53: goto B4;
            case 54: goto B4;
            case 55: goto B26;
            case 56: goto B4;
            case 57: goto B28;
            default: goto B4;
          }
          B31:;
          i0 = 4294967294u;
          i1 = 4294967295u;
          i2 = l2;
          i2 = i32_load8_s(Z_envZ_memory, (u64)(i2));
          i3 = 104u;
          i2 = i2 == i3;
          l8 = i2;
          i0 = i2 ? i0 : i1;
          l11 = i0;
          i0 = l4;
          i1 = 2u;
          i0 += i1;
          i1 = l2;
          i2 = l8;
          i0 = i2 ? i0 : i1;
          l2 = i0;
          goto B25;
          B30:;
          i0 = 3u;
          i1 = 1u;
          i2 = l2;
          i2 = i32_load8_s(Z_envZ_memory, (u64)(i2));
          i3 = 108u;
          i2 = i2 == i3;
          l8 = i2;
          i0 = i2 ? i0 : i1;
          l11 = i0;
          i0 = l4;
          i1 = 2u;
          i0 += i1;
          i1 = l2;
          i2 = l8;
          i0 = i2 ? i0 : i1;
          l2 = i0;
          goto B25;
          B29:;
          i0 = 3u;
          l11 = i0;
          goto B25;
          B28:;
          i0 = 1u;
          l11 = i0;
          goto B25;
          B27:;
          i0 = 2u;
          l11 = i0;
          goto B25;
          B26:;
          i0 = 0u;
          l11 = i0;
          i0 = l4;
          l2 = i0;
          B25:;
          i0 = 1u;
          i1 = l11;
          i2 = l2;
          i2 = i32_load8_u(Z_envZ_memory, (u64)(i2));
          l4 = i2;
          i3 = 47u;
          i2 &= i3;
          i3 = 3u;
          i2 = i2 == i3;
          l11 = i2;
          i0 = i2 ? i0 : i1;
          l16 = i0;
          i0 = l4;
          i1 = 32u;
          i0 |= i1;
          i1 = l4;
          i2 = l11;
          i0 = i2 ? i0 : i1;
          l15 = i0;
          i1 = 255u;
          i0 &= i1;
          l8 = i0;
          i1 = 24u;
          i0 <<= (i1 & 31);
          i1 = 24u;
          i0 = (u32)((s32)i0 >> (i1 & 31));
          i1 = 91u;
          i0 -= i1;
          switch (i0) {
            case 0: goto B35;
            case 1: goto B33;
            case 2: goto B33;
            case 3: goto B33;
            case 4: goto B33;
            case 5: goto B33;
            case 6: goto B33;
            case 7: goto B33;
            case 8: goto B36;
            case 9: goto B33;
            case 10: goto B33;
            case 11: goto B33;
            case 12: goto B33;
            case 13: goto B33;
            case 14: goto B33;
            case 15: goto B33;
            case 16: goto B33;
            case 17: goto B33;
            case 18: goto B33;
            case 19: goto B34;
            default: goto B33;
          }
          B36:;
          i0 = l14;
          i1 = 1u;
          i2 = l14;
          i3 = 1u;
          i2 = (u32)((s32)i2 > (s32)i3);
          i0 = i2 ? i0 : i1;
          l14 = i0;
          i0 = l3;
          goto B32;
          B35:;
          i0 = l3;
          goto B32;
          B34:;
          i0 = l9;
          i1 = l16;
          i2 = l3;
          j2 = (u64)(s64)(s32)(i2);
          f56(i0, i1, j2);
          goto B14;
          B33:;
          i0 = p0;
          i1 = 0u;
          f30(i0, i1);
          L37: 
            i0 = l5;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
            l4 = i0;
            i1 = l13;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            i0 = i0 < i1;
            if (i0) {
              i0 = l5;
              i1 = l4;
              i2 = 1u;
              i1 += i2;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
              i0 = l4;
              i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
            } else {
              i0 = p0;
              i0 = f32_0(i0);
            }
            i0 = f33(i0);
            if (i0) {goto L37;}
          i0 = l13;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          if (i0) {
            i0 = l5;
            i1 = l5;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            i2 = 4294967295u;
            i1 += i2;
            l4 = i1;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
          } else {
            i0 = l5;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
            l4 = i0;
          }
          i0 = l3;
          i1 = l17;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i0 += i1;
          i1 = l4;
          i0 += i1;
          i1 = l18;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i0 -= i1;
          B32:;
          l11 = i0;
          i0 = p0;
          i1 = l14;
          f30(i0, i1);
          i0 = l5;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l4 = i0;
          i1 = l13;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          l3 = i1;
          i0 = i0 < i1;
          if (i0) {
            i0 = l5;
            i1 = l4;
            i2 = 1u;
            i1 += i2;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
          } else {
            i0 = p0;
            i0 = f32_0(i0);
            i1 = 0u;
            i0 = (u32)((s32)i0 < (s32)i1);
            if (i0) {goto B4;}
            i0 = l13;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
            l3 = i0;
          }
          i0 = l3;
          if (i0) {
            i0 = l5;
            i1 = l5;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            i2 = 4294967295u;
            i1 += i2;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
          }
          i0 = l8;
          i1 = 24u;
          i0 <<= (i1 & 31);
          i1 = 24u;
          i0 = (u32)((s32)i0 >> (i1 & 31));
          i1 = 65u;
          i0 -= i1;
          switch (i0) {
            case 0: goto B44;
            case 1: goto B42;
            case 2: goto B42;
            case 3: goto B42;
            case 4: goto B44;
            case 5: goto B44;
            case 6: goto B44;
            case 7: goto B42;
            case 8: goto B42;
            case 9: goto B42;
            case 10: goto B42;
            case 11: goto B42;
            case 12: goto B42;
            case 13: goto B42;
            case 14: goto B42;
            case 15: goto B42;
            case 16: goto B42;
            case 17: goto B42;
            case 18: goto B42;
            case 19: goto B42;
            case 20: goto B42;
            case 21: goto B42;
            case 22: goto B42;
            case 23: goto B48;
            case 24: goto B42;
            case 25: goto B42;
            case 26: goto B49;
            case 27: goto B42;
            case 28: goto B42;
            case 29: goto B42;
            case 30: goto B42;
            case 31: goto B42;
            case 32: goto B44;
            case 33: goto B42;
            case 34: goto B49;
            case 35: goto B46;
            case 36: goto B44;
            case 37: goto B44;
            case 38: goto B44;
            case 39: goto B42;
            case 40: goto B45;
            case 41: goto B42;
            case 42: goto B42;
            case 43: goto B42;
            case 44: goto B42;
            case 45: goto B42;
            case 46: goto B47;
            case 47: goto B48;
            case 48: goto B42;
            case 49: goto B42;
            case 50: goto B49;
            case 51: goto B42;
            case 52: goto B46;
            case 53: goto B42;
            case 54: goto B42;
            case 55: goto B48;
            default: goto B42;
          }
          B49:;
          i0 = l15;
          i1 = 16u;
          i0 |= i1;
          i1 = 115u;
          i0 = i0 == i1;
          if (i0) {
            i0 = l12;
            i1 = 4294967295u;
            i2 = 257u;
            i0 = _memset(i0, i1, i2);
            i0 = l12;
            i1 = 0u;
            i32_store8(Z_envZ_memory, (u64)(i0), i1);
            i0 = l15;
            i1 = 115u;
            i0 = i0 == i1;
            if (i0) {
              i0 = l26;
              i1 = 0u;
              i32_store8(Z_envZ_memory, (u64)(i0), i1);
              i0 = l24;
              i1 = 0u;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
              i0 = l24;
              i1 = 0u;
              i32_store8(Z_envZ_memory, (u64)(i0 + 4), i1);
            }
          } else {
            i0 = l12;
            i1 = l2;
            i2 = 1u;
            i1 += i2;
            l4 = i1;
            i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
            i2 = 94u;
            i1 = i1 == i2;
            l8 = i1;
            l3 = i1;
            i2 = 257u;
            i0 = _memset(i0, i1, i2);
            i0 = l12;
            i1 = 0u;
            i32_store8(Z_envZ_memory, (u64)(i0), i1);
            i0 = l2;
            i1 = 2u;
            i0 += i1;
            i1 = l4;
            i2 = l8;
            i0 = i2 ? i0 : i1;
            l2 = i0;
            i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
            i1 = 45u;
            i0 -= i1;
            switch (i0) {
              case 0: goto B56;
              case 1: goto B54;
              case 2: goto B54;
              case 3: goto B54;
              case 4: goto B54;
              case 5: goto B54;
              case 6: goto B54;
              case 7: goto B54;
              case 8: goto B54;
              case 9: goto B54;
              case 10: goto B54;
              case 11: goto B54;
              case 12: goto B54;
              case 13: goto B54;
              case 14: goto B54;
              case 15: goto B54;
              case 16: goto B54;
              case 17: goto B54;
              case 18: goto B54;
              case 19: goto B54;
              case 20: goto B54;
              case 21: goto B54;
              case 22: goto B54;
              case 23: goto B54;
              case 24: goto B54;
              case 25: goto B54;
              case 26: goto B54;
              case 27: goto B54;
              case 28: goto B54;
              case 29: goto B54;
              case 30: goto B54;
              case 31: goto B54;
              case 32: goto B54;
              case 33: goto B54;
              case 34: goto B54;
              case 35: goto B54;
              case 36: goto B54;
              case 37: goto B54;
              case 38: goto B54;
              case 39: goto B54;
              case 40: goto B54;
              case 41: goto B54;
              case 42: goto B54;
              case 43: goto B54;
              case 44: goto B54;
              case 45: goto B54;
              case 46: goto B54;
              case 47: goto B54;
              case 48: goto B55;
              default: goto B54;
            }
            B56:;
            i0 = l27;
            i1 = l3;
            i2 = 1u;
            i1 ^= i2;
            i2 = 255u;
            i1 &= i2;
            l4 = i1;
            i32_store8(Z_envZ_memory, (u64)(i0), i1);
            i0 = l2;
            i1 = 1u;
            i0 += i1;
            l2 = i0;
            goto B53;
            B55:;
            i0 = l28;
            i1 = l3;
            i2 = 1u;
            i1 ^= i2;
            i2 = 255u;
            i1 &= i2;
            l4 = i1;
            i32_store8(Z_envZ_memory, (u64)(i0), i1);
            i0 = l2;
            i1 = 1u;
            i0 += i1;
            l2 = i0;
            goto B53;
            B54:;
            i0 = l3;
            i1 = 1u;
            i0 ^= i1;
            i1 = 255u;
            i0 &= i1;
            l4 = i0;
            B53:;
            L57: 
              i0 = l2;
              i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
              l3 = i0;
              switch (i0) {
                case 0: goto B4;
                case 1: goto B58;
                case 2: goto B58;
                case 3: goto B58;
                case 4: goto B58;
                case 5: goto B58;
                case 6: goto B58;
                case 7: goto B58;
                case 8: goto B58;
                case 9: goto B58;
                case 10: goto B58;
                case 11: goto B58;
                case 12: goto B58;
                case 13: goto B58;
                case 14: goto B58;
                case 15: goto B58;
                case 16: goto B58;
                case 17: goto B58;
                case 18: goto B58;
                case 19: goto B58;
                case 20: goto B58;
                case 21: goto B58;
                case 22: goto B58;
                case 23: goto B58;
                case 24: goto B58;
                case 25: goto B58;
                case 26: goto B58;
                case 27: goto B58;
                case 28: goto B58;
                case 29: goto B58;
                case 30: goto B58;
                case 31: goto B58;
                case 32: goto B58;
                case 33: goto B58;
                case 34: goto B58;
                case 35: goto B58;
                case 36: goto B58;
                case 37: goto B58;
                case 38: goto B58;
                case 39: goto B58;
                case 40: goto B58;
                case 41: goto B58;
                case 42: goto B58;
                case 43: goto B58;
                case 44: goto B58;
                case 45: goto B59;
                case 46: goto B58;
                case 47: goto B58;
                case 48: goto B58;
                case 49: goto B58;
                case 50: goto B58;
                case 51: goto B58;
                case 52: goto B58;
                case 53: goto B58;
                case 54: goto B58;
                case 55: goto B58;
                case 56: goto B58;
                case 57: goto B58;
                case 58: goto B58;
                case 59: goto B58;
                case 60: goto B58;
                case 61: goto B58;
                case 62: goto B58;
                case 63: goto B58;
                case 64: goto B58;
                case 65: goto B58;
                case 66: goto B58;
                case 67: goto B58;
                case 68: goto B58;
                case 69: goto B58;
                case 70: goto B58;
                case 71: goto B58;
                case 72: goto B58;
                case 73: goto B58;
                case 74: goto B58;
                case 75: goto B58;
                case 76: goto B58;
                case 77: goto B58;
                case 78: goto B58;
                case 79: goto B58;
                case 80: goto B58;
                case 81: goto B58;
                case 82: goto B58;
                case 83: goto B58;
                case 84: goto B58;
                case 85: goto B58;
                case 86: goto B58;
                case 87: goto B58;
                case 88: goto B58;
                case 89: goto B58;
                case 90: goto B58;
                case 91: goto B58;
                case 92: goto B58;
                case 93: goto B52;
                default: goto B58;
              }
              B59:;
              i0 = l2;
              i1 = 1u;
              i0 += i1;
              l3 = i0;
              i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
              l8 = i0;
              switch (i0) {
                case 0: goto B61;
                case 1: goto B60;
                case 2: goto B60;
                case 3: goto B60;
                case 4: goto B60;
                case 5: goto B60;
                case 6: goto B60;
                case 7: goto B60;
                case 8: goto B60;
                case 9: goto B60;
                case 10: goto B60;
                case 11: goto B60;
                case 12: goto B60;
                case 13: goto B60;
                case 14: goto B60;
                case 15: goto B60;
                case 16: goto B60;
                case 17: goto B60;
                case 18: goto B60;
                case 19: goto B60;
                case 20: goto B60;
                case 21: goto B60;
                case 22: goto B60;
                case 23: goto B60;
                case 24: goto B60;
                case 25: goto B60;
                case 26: goto B60;
                case 27: goto B60;
                case 28: goto B60;
                case 29: goto B60;
                case 30: goto B60;
                case 31: goto B60;
                case 32: goto B60;
                case 33: goto B60;
                case 34: goto B60;
                case 35: goto B60;
                case 36: goto B60;
                case 37: goto B60;
                case 38: goto B60;
                case 39: goto B60;
                case 40: goto B60;
                case 41: goto B60;
                case 42: goto B60;
                case 43: goto B60;
                case 44: goto B60;
                case 45: goto B60;
                case 46: goto B60;
                case 47: goto B60;
                case 48: goto B60;
                case 49: goto B60;
                case 50: goto B60;
                case 51: goto B60;
                case 52: goto B60;
                case 53: goto B60;
                case 54: goto B60;
                case 55: goto B60;
                case 56: goto B60;
                case 57: goto B60;
                case 58: goto B60;
                case 59: goto B60;
                case 60: goto B60;
                case 61: goto B60;
                case 62: goto B60;
                case 63: goto B60;
                case 64: goto B60;
                case 65: goto B60;
                case 66: goto B60;
                case 67: goto B60;
                case 68: goto B60;
                case 69: goto B60;
                case 70: goto B60;
                case 71: goto B60;
                case 72: goto B60;
                case 73: goto B60;
                case 74: goto B60;
                case 75: goto B60;
                case 76: goto B60;
                case 77: goto B60;
                case 78: goto B60;
                case 79: goto B60;
                case 80: goto B60;
                case 81: goto B60;
                case 82: goto B60;
                case 83: goto B60;
                case 84: goto B60;
                case 85: goto B60;
                case 86: goto B60;
                case 87: goto B60;
                case 88: goto B60;
                case 89: goto B60;
                case 90: goto B60;
                case 91: goto B60;
                case 92: goto B60;
                case 93: goto B61;
                default: goto B60;
              }
              B61:;
              i0 = 45u;
              l3 = i0;
              goto B58;
              B60:;
              i0 = l2;
              i1 = 4294967295u;
              i0 += i1;
              i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
              l2 = i0;
              i1 = l8;
              i2 = 255u;
              i1 &= i2;
              i0 = (u32)((s32)i0 < (s32)i1);
              if (i0) {
                i0 = l2;
                i1 = 255u;
                i0 &= i1;
                l2 = i0;
                L63: 
                  i0 = l2;
                  i1 = 1u;
                  i0 += i1;
                  l2 = i0;
                  i1 = l12;
                  i0 += i1;
                  i1 = l4;
                  i32_store8(Z_envZ_memory, (u64)(i0), i1);
                  i0 = l2;
                  i1 = l3;
                  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
                  l8 = i1;
                  i2 = 255u;
                  i1 &= i2;
                  i0 = i0 < i1;
                  if (i0) {goto L63;}
                  i0 = l3;
                  l2 = i0;
                  i0 = l8;
              } else {
                i0 = l3;
                l2 = i0;
                i0 = l8;
              }
              l3 = i0;
              B58:;
              i0 = l3;
              i1 = 255u;
              i0 &= i1;
              i1 = 1u;
              i0 += i1;
              i1 = l12;
              i0 += i1;
              i1 = l4;
              i32_store8(Z_envZ_memory, (u64)(i0), i1);
              i0 = l2;
              i1 = 1u;
              i0 += i1;
              l2 = i0;
              goto L57;
            UNREACHABLE;
            B52:;
          }
          i0 = l14;
          i1 = 1u;
          i0 += i1;
          i1 = 31u;
          i2 = l15;
          i3 = 99u;
          i2 = i2 == i3;
          l25 = i2;
          i0 = i2 ? i0 : i1;
          l3 = i0;
          i0 = l7;
          i1 = 0u;
          i0 = i0 != i1;
          l15 = i0;
          i0 = l16;
          i1 = 1u;
          i0 = i0 == i1;
          l16 = i0;
          if (i0) {
            i0 = l15;
            if (i0) {
              i0 = l3;
              i1 = 2u;
              i0 <<= (i1 & 31);
              i0 = _malloc(i0);
              l10 = i0;
              i0 = !(i0);
              if (i0) {
                i0 = 0u;
                l6 = i0;
                i0 = 0u;
                l10 = i0;
                goto B4;
              }
            } else {
              i0 = l9;
              l10 = i0;
            }
            i0 = l19;
            i1 = 0u;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            i0 = l29;
            i1 = 0u;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            i0 = 0u;
            l6 = i0;
            L67: 
              i0 = l10;
              i0 = !(i0);
              l8 = i0;
              L69: 
                L70: 
                  i0 = l5;
                  i0 = i32_load(Z_envZ_memory, (u64)(i0));
                  l4 = i0;
                  i1 = l13;
                  i1 = i32_load(Z_envZ_memory, (u64)(i1));
                  i0 = i0 < i1;
                  if (i0) {
                    i0 = l5;
                    i1 = l4;
                    i2 = 1u;
                    i1 += i2;
                    i32_store(Z_envZ_memory, (u64)(i0), i1);
                    i0 = l4;
                    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
                  } else {
                    i0 = p0;
                    i0 = f32_0(i0);
                  }
                  l4 = i0;
                  i1 = 1u;
                  i0 += i1;
                  i1 = l12;
                  i0 += i1;
                  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
                  i0 = !(i0);
                  if (i0) {goto B68;}
                  i0 = l23;
                  i1 = l4;
                  i32_store8(Z_envZ_memory, (u64)(i0), i1);
                  i0 = l22;
                  i1 = l23;
                  i2 = l19;
                  i0 = f37(i0, i1, i2);
                  i1 = 4294967294u;
                  i0 -= i1;
                  switch (i0) {
                    case 0: goto B73;
                    case 1: goto B74;
                    default: goto B71;
                  }
                  B74:;
                  i0 = 0u;
                  l6 = i0;
                  goto B4;
                  B73:;
                  goto L70;
                  B71:;
                i0 = l8;
                i0 = !(i0);
                if (i0) {
                  i0 = l6;
                  i1 = 2u;
                  i0 <<= (i1 & 31);
                  i1 = l10;
                  i0 += i1;
                  i1 = l22;
                  i1 = i32_load(Z_envZ_memory, (u64)(i1));
                  i32_store(Z_envZ_memory, (u64)(i0), i1);
                  i0 = l6;
                  i1 = 1u;
                  i0 += i1;
                  l6 = i0;
                }
                i0 = l15;
                i1 = l3;
                i2 = l6;
                i1 = i1 == i2;
                i0 &= i1;
                i0 = !(i0);
                if (i0) {goto L69;}
              i0 = l10;
              i1 = l3;
              i2 = 1u;
              i1 <<= (i2 & 31);
              i2 = 1u;
              i1 |= i2;
              l3 = i1;
              i2 = 2u;
              i1 <<= (i2 & 31);
              i0 = f19(i0, i1);
              l4 = i0;
              if (i0) {
                i0 = l4;
                l10 = i0;
                goto L67;
              } else {
                i0 = 0u;
                l6 = i0;
                goto B4;
              }
              UNREACHABLE;
              B68:;
            i0 = l19;
            l4 = i0;
            if (i0) {
              i0 = l4;
              i0 = i32_load(Z_envZ_memory, (u64)(i0));
              i0 = !(i0);
            } else {
              i0 = 1u;
            }
            if (i0) {
              i0 = l6;
              l3 = i0;
              i0 = l10;
              l4 = i0;
              i0 = 0u;
            } else {
              i0 = 0u;
              l6 = i0;
              goto B4;
            }
            l6 = i0;
          } else {
            i0 = l15;
            if (i0) {
              i0 = l3;
              i0 = _malloc(i0);
              l6 = i0;
              i0 = !(i0);
              if (i0) {
                i0 = 0u;
                l6 = i0;
                i0 = 0u;
                l10 = i0;
                goto B4;
              }
              i0 = 0u;
              l10 = i0;
              L82: 
                L83: 
                  i0 = l5;
                  i0 = i32_load(Z_envZ_memory, (u64)(i0));
                  l4 = i0;
                  i1 = l13;
                  i1 = i32_load(Z_envZ_memory, (u64)(i1));
                  i0 = i0 < i1;
                  if (i0) {
                    i0 = l5;
                    i1 = l4;
                    i2 = 1u;
                    i1 += i2;
                    i32_store(Z_envZ_memory, (u64)(i0), i1);
                    i0 = l4;
                    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
                  } else {
                    i0 = p0;
                    i0 = f32_0(i0);
                  }
                  l4 = i0;
                  i1 = 1u;
                  i0 += i1;
                  i1 = l12;
                  i0 += i1;
                  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
                  i0 = !(i0);
                  if (i0) {
                    i0 = l10;
                    l3 = i0;
                    i0 = 0u;
                    l4 = i0;
                    i0 = 0u;
                    l10 = i0;
                    goto B79;
                  }
                  i0 = l6;
                  i1 = l10;
                  i0 += i1;
                  i1 = l4;
                  i32_store8(Z_envZ_memory, (u64)(i0), i1);
                  i0 = l10;
                  i1 = 1u;
                  i0 += i1;
                  l10 = i0;
                  i1 = l3;
                  i0 = i0 != i1;
                  if (i0) {goto L83;}
                i0 = l6;
                i1 = l3;
                i2 = 1u;
                i1 <<= (i2 & 31);
                i2 = 1u;
                i1 |= i2;
                l3 = i1;
                i0 = f19(i0, i1);
                l4 = i0;
                if (i0) {
                  i0 = l4;
                  l6 = i0;
                  goto L82;
                } else {
                  i0 = 0u;
                  l10 = i0;
                  goto B4;
                }
                UNREACHABLE;
              UNREACHABLE;
            }
            i0 = l9;
            i0 = !(i0);
            if (i0) {
              L88: 
                i0 = l5;
                i0 = i32_load(Z_envZ_memory, (u64)(i0));
                l6 = i0;
                i1 = l13;
                i1 = i32_load(Z_envZ_memory, (u64)(i1));
                i0 = i0 < i1;
                if (i0) {
                  i0 = l5;
                  i1 = l6;
                  i2 = 1u;
                  i1 += i2;
                  i32_store(Z_envZ_memory, (u64)(i0), i1);
                  i0 = l6;
                  i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
                } else {
                  i0 = p0;
                  i0 = f32_0(i0);
                }
                i1 = 1u;
                i0 += i1;
                i1 = l12;
                i0 += i1;
                i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
                if (i0) {goto L88;}
                i0 = 0u;
                l3 = i0;
                i0 = 0u;
                l6 = i0;
                i0 = 0u;
                l4 = i0;
                i0 = 0u;
                l10 = i0;
                goto B79;
              UNREACHABLE;
            }
            i0 = 0u;
            l3 = i0;
            L90: 
              i0 = l5;
              i0 = i32_load(Z_envZ_memory, (u64)(i0));
              l6 = i0;
              i1 = l13;
              i1 = i32_load(Z_envZ_memory, (u64)(i1));
              i0 = i0 < i1;
              if (i0) {
                i0 = l5;
                i1 = l6;
                i2 = 1u;
                i1 += i2;
                i32_store(Z_envZ_memory, (u64)(i0), i1);
                i0 = l6;
                i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
              } else {
                i0 = p0;
                i0 = f32_0(i0);
              }
              l6 = i0;
              i1 = 1u;
              i0 += i1;
              i1 = l12;
              i0 += i1;
              i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
              if (i0) {
                i0 = l3;
                i1 = l9;
                i0 += i1;
                i1 = l6;
                i32_store8(Z_envZ_memory, (u64)(i0), i1);
                i0 = l3;
                i1 = 1u;
                i0 += i1;
                l3 = i0;
                goto L90;
              } else {
                i0 = 0u;
                l4 = i0;
                i0 = 0u;
                l10 = i0;
                i0 = l9;
              }
            l6 = i0;
            B79:;
          }
          i0 = l13;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          if (i0) {
            i0 = l5;
            i1 = l5;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            i2 = 4294967295u;
            i1 += i2;
            l8 = i1;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
          } else {
            i0 = l5;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
            l8 = i0;
          }
          i0 = l17;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          i1 = l8;
          i2 = l18;
          i2 = i32_load(Z_envZ_memory, (u64)(i2));
          i1 -= i2;
          i0 += i1;
          l8 = i0;
          i0 = !(i0);
          if (i0) {goto B7;}
          i0 = l25;
          i1 = 1u;
          i0 ^= i1;
          i1 = l8;
          i2 = l14;
          i1 = i1 == i2;
          i0 |= i1;
          i0 = !(i0);
          if (i0) {goto B7;}
          i0 = l15;
          if (i0) {
            i0 = l16;
            if (i0) {
              i0 = l9;
              i1 = l4;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
            } else {
              i0 = l9;
              i1 = l6;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
            }
          }
          i0 = l25;
          i0 = !(i0);
          if (i0) {
            i0 = l4;
            if (i0) {
              i0 = l3;
              i1 = 2u;
              i0 <<= (i1 & 31);
              i1 = l4;
              i0 += i1;
              i1 = 0u;
              i32_store(Z_envZ_memory, (u64)(i0), i1);
            }
            i0 = l6;
            i0 = !(i0);
            if (i0) {
              i0 = 0u;
              l6 = i0;
              goto B42;
            }
            i0 = l3;
            i1 = l6;
            i0 += i1;
            i1 = 0u;
            i32_store8(Z_envZ_memory, (u64)(i0), i1);
          }
          goto B42;
          B48:;
          i0 = 16u;
          l3 = i0;
          goto B43;
          B47:;
          i0 = 8u;
          l3 = i0;
          goto B43;
          B46:;
          i0 = 10u;
          l3 = i0;
          goto B43;
          B45:;
          i0 = 0u;
          l3 = i0;
          goto B43;
          B44:;
          i0 = p0;
          i1 = l16;
          d0 = f57(i0, i1);
          l31 = d0;
          i0 = l17;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          i1 = l18;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i2 = l5;
          i2 = i32_load(Z_envZ_memory, (u64)(i2));
          i1 -= i2;
          i0 = i0 == i1;
          if (i0) {goto B7;}
          i0 = l9;
          if (i0) {
            i0 = l16;
            switch (i0) {
              case 0: goto B102;
              case 1: goto B101;
              case 2: goto B100;
              default: goto B42;
            }
            B102:;
            i0 = l9;
            d1 = l31;
            f1 = (f32)(d1);
            f32_store(Z_envZ_memory, (u64)(i0), f1);
            goto B42;
            B101:;
            i0 = l9;
            d1 = l31;
            f64_store(Z_envZ_memory, (u64)(i0), d1);
            goto B42;
            B100:;
            i0 = l9;
            d1 = l31;
            f64_store(Z_envZ_memory, (u64)(i0), d1);
            goto B42;
          }
          goto B42;
          B43:;
          i0 = p0;
          i1 = l3;
          j0 = f31(i0, i1);
          l30 = j0;
          i0 = l17;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          i1 = l18;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i2 = l5;
          i2 = i32_load(Z_envZ_memory, (u64)(i2));
          i1 -= i2;
          i0 = i0 == i1;
          if (i0) {goto B7;}
          i0 = l15;
          i1 = 112u;
          i0 = i0 == i1;
          i1 = l9;
          i2 = 0u;
          i1 = i1 != i2;
          i0 &= i1;
          if (i0) {
            i0 = l9;
            j1 = l30;
            i64_store32(Z_envZ_memory, (u64)(i0), j1);
          } else {
            i0 = l9;
            i1 = l16;
            j2 = l30;
            f56(i0, i1, j2);
          }
          B42:;
          i0 = l21;
          i1 = l9;
          i2 = 0u;
          i1 = i1 != i2;
          i0 += i1;
          l21 = i0;
          i0 = l5;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          i1 = l11;
          i2 = l17;
          i2 = i32_load(Z_envZ_memory, (u64)(i2));
          i1 += i2;
          i0 += i1;
          i1 = l18;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i0 -= i1;
          l3 = i0;
          goto B14;
          B16:;
        }
        i0 = p0;
        i1 = 0u;
        f30(i0, i1);
        i0 = l5;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l7 = i0;
        i1 = l13;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i0 = i0 < i1;
        if (i0) {
          i0 = l5;
          i1 = l7;
          i2 = 1u;
          i1 += i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l7;
          i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
        } else {
          i0 = p0;
          i0 = f32_0(i0);
        }
        l7 = i0;
        i0 = l7;
        i1 = l2;
        i2 = l9;
        i1 += i2;
        l2 = i1;
        i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
        i0 = i0 != i1;
        if (i0) {goto B5;}
        i0 = l3;
        i1 = 1u;
        i0 += i1;
        l3 = i0;
        B14:;
      }
      i0 = l2;
      i1 = 1u;
      i0 += i1;
      l2 = i0;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      l7 = i0;
      if (i0) {goto L6;}
      goto B2;
      B7:;
    goto B3;
    B5:;
    i0 = l13;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    if (i0) {
      i0 = l5;
      i1 = l5;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 4294967295u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    i0 = l7;
    i1 = 4294967295u;
    i0 = (u32)((s32)i0 > (s32)i1);
    i1 = l21;
    i0 |= i1;
    if (i0) {goto B2;}
    i0 = 0u;
    l7 = i0;
    goto B3;
    B4:;
    i0 = l21;
    i0 = !(i0);
    if (i0) {goto B3;}
    B3:;
    i0 = l7;
    if (i0) {
      i0 = l6;
      _free(i0);
      i0 = l10;
      _free(i0);
    }
    B2:;
  }
  i0 = l20;
  g7 = i0;
  FUNC_EPILOGUE;
}

static u32 f55(u32 p0, u32 p1) {
  u32 l2 = 0, l3 = 0, l4 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = g7;
  l2 = i0;
  i0 = g7;
  i1 = 16u;
  i0 += i1;
  g7 = i0;
  i0 = l2;
  l3 = i0;
  i1 = p0;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  L0: 
    i0 = l3;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i1 = 3u;
    i0 += i1;
    i1 = 4294967292u;
    i0 &= i1;
    p0 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l4 = i0;
    i0 = l3;
    i1 = p0;
    i2 = 4u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    i1 = 4294967295u;
    i0 += i1;
    p0 = i0;
    i0 = p1;
    i1 = 1u;
    i0 = i0 > i1;
    if (i0) {
      i0 = p0;
      p1 = i0;
      goto L0;
    }
  i0 = l2;
  g7 = i0;
  i0 = l4;
  FUNC_EPILOGUE;
  return i0;
}

static void f56(u32 p0, u32 p1, u64 p2) {
  FUNC_PROLOGUE;
  u32 i0, i1;
  u64 j1;
  i0 = p0;
  if (i0) {
    i0 = p1;
    i1 = 4294967294u;
    i0 -= i1;
    switch (i0) {
      case 0: goto B6;
      case 1: goto B5;
      case 2: goto B4;
      case 3: goto B3;
      case 4: goto B1;
      case 5: goto B2;
      default: goto B1;
    }
    B6:;
    i0 = p0;
    j1 = p2;
    i64_store8(Z_envZ_memory, (u64)(i0), j1);
    goto B1;
    B5:;
    i0 = p0;
    j1 = p2;
    i64_store16(Z_envZ_memory, (u64)(i0), j1);
    goto B1;
    B4:;
    i0 = p0;
    j1 = p2;
    i64_store32(Z_envZ_memory, (u64)(i0), j1);
    goto B1;
    B3:;
    i0 = p0;
    j1 = p2;
    i64_store32(Z_envZ_memory, (u64)(i0), j1);
    goto B1;
    B2:;
    i0 = p0;
    j1 = p2;
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    B1:;
  }
  FUNC_EPILOGUE;
}

static f64 f57(u32 p0, u32 p1) {
  u32 l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  f32 f0, f1;
  f64 d0, d1;
  i0 = p1;
  switch (i0) {
    case 0: goto B5;
    case 1: goto B4;
    case 2: goto B3;
    default: goto B2;
  }
  B5:;
  i0 = 4294967147u;
  l5 = i0;
  i0 = 24u;
  l6 = i0;
  goto B1;
  B4:;
  i0 = 4294966222u;
  l5 = i0;
  i0 = 53u;
  l6 = i0;
  goto B1;
  B3:;
  i0 = 4294966222u;
  l5 = i0;
  i0 = 53u;
  l6 = i0;
  goto B1;
  B2:;
  d0 = 0;
  goto B0;
  B1:;
  i0 = p0;
  i1 = 4u;
  i0 += i1;
  l2 = i0;
  i0 = p0;
  i1 = 100u;
  i0 += i1;
  l4 = i0;
  L6: 
    i0 = l2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    p1 = i0;
    i1 = l4;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 < i1;
    if (i0) {
      i0 = l2;
      i1 = p1;
      i2 = 1u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p1;
      i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    } else {
      i0 = p0;
      i0 = f32_0(i0);
    }
    p1 = i0;
    i0 = f33(i0);
    if (i0) {goto L6;}
  i0 = p1;
  i1 = 43u;
  i0 -= i1;
  switch (i0) {
    case 0: goto B10;
    case 1: goto B9;
    case 2: goto B10;
    default: goto B9;
  }
  B10:;
  i0 = 1u;
  i1 = p1;
  i2 = 45u;
  i1 = i1 == i2;
  i2 = 1u;
  i1 <<= (i2 & 31);
  i0 -= i1;
  l7 = i0;
  i0 = l2;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  p1 = i0;
  i1 = l4;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i0 = i0 < i1;
  if (i0) {
    i0 = l2;
    i1 = p1;
    i2 = 1u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    p1 = i0;
    goto B8;
  } else {
    i0 = p0;
    i0 = f32_0(i0);
    p1 = i0;
    goto B8;
  }
  UNREACHABLE;
  B9:;
  i0 = 1u;
  l7 = i0;
  B8:;
  L15: 
    i0 = l3;
    i1 = 2603u;
    i0 += i1;
    i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
    i1 = p1;
    i2 = 32u;
    i1 |= i2;
    i0 = i0 == i1;
    if (i0) {
      i0 = l3;
      i1 = 7u;
      i0 = i0 < i1;
      if (i0) {
        i0 = l2;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        p1 = i0;
        i1 = l4;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i0 = i0 < i1;
        if (i0) {
          i0 = l2;
          i1 = p1;
          i2 = 1u;
          i1 += i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = p1;
          i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
        } else {
          i0 = p0;
          i0 = f32_0(i0);
        }
        p1 = i0;
      }
      i0 = l3;
      i1 = 1u;
      i0 += i1;
      l3 = i0;
      i1 = 8u;
      i0 = i0 < i1;
      if (i0) {goto L15;}
      i0 = 8u;
    } else {
      i0 = l3;
    }
  l3 = i0;
  i1 = 2147483647u;
  i0 &= i1;
  i1 = 3u;
  i0 -= i1;
  switch (i0) {
    case 0: goto B13;
    case 1: goto B14;
    case 2: goto B14;
    case 3: goto B14;
    case 4: goto B14;
    case 5: goto B12;
    default: goto B14;
  }
  B14:;
  i0 = l3;
  i0 = !(i0);
  if (i0) {
    i0 = 0u;
    l3 = i0;
    L21: 
      i0 = l3;
      i1 = 2612u;
      i0 += i1;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      i1 = p1;
      i2 = 32u;
      i1 |= i2;
      i0 = i0 != i1;
      if (i0) {goto B20;}
      i0 = l3;
      i1 = 2u;
      i0 = i0 < i1;
      if (i0) {
        i0 = l2;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        p1 = i0;
        i1 = l4;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i0 = i0 < i1;
        if (i0) {
          i0 = l2;
          i1 = p1;
          i2 = 1u;
          i1 += i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = p1;
          i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
        } else {
          i0 = p0;
          i0 = f32_0(i0);
        }
        p1 = i0;
      }
      i0 = l3;
      i1 = 1u;
      i0 += i1;
      l3 = i0;
      i1 = 3u;
      i0 = i0 < i1;
      if (i0) {goto L21;}
      i0 = 3u;
    l3 = i0;
    B20:;
  }
  i0 = l3;
  switch (i0) {
    case 0: goto B25;
    case 1: goto B24;
    case 2: goto B24;
    case 3: goto B26;
    default: goto B24;
  }
  B26:;
  i0 = l2;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  p1 = i0;
  i1 = l4;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i0 = i0 < i1;
  if (i0) {
    i0 = l2;
    i1 = p1;
    i2 = 1u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
  } else {
    i0 = p0;
    i0 = f32_0(i0);
  }
  i1 = 40u;
  i0 = i0 != i1;
  if (i0) {
    d0 = g5;
    i1 = l4;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i1 = !(i1);
    if (i1) {goto B0;}
    i0 = l2;
    i1 = l2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 4294967295u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    d0 = g5;
    goto B0;
  }
  i0 = 1u;
  p1 = i0;
  L29: 
    i0 = l2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    i1 = l4;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 < i1;
    if (i0) {
      i0 = l2;
      i1 = l3;
      i2 = 1u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    } else {
      i0 = p0;
      i0 = f32_0(i0);
    }
    l3 = i0;
    i1 = 4294967248u;
    i0 += i1;
    i1 = 10u;
    i0 = i0 < i1;
    i1 = l3;
    i2 = 4294967231u;
    i1 += i2;
    i2 = 26u;
    i1 = i1 < i2;
    i0 |= i1;
    i0 = !(i0);
    if (i0) {
      i0 = l3;
      i1 = 95u;
      i0 = i0 == i1;
      i1 = l3;
      i2 = 4294967199u;
      i1 += i2;
      i2 = 26u;
      i1 = i1 < i2;
      i0 |= i1;
      i0 = !(i0);
      if (i0) {goto B30;}
    }
    i0 = p1;
    i1 = 1u;
    i0 += i1;
    p1 = i0;
    goto L29;
    B30:;
  d0 = g5;
  i1 = l3;
  i2 = 41u;
  i1 = i1 == i2;
  if (i1) {goto B0;}
  i0 = l4;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  if (i0) {
    i0 = l2;
    i1 = l2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 4294967295u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  }
  i0 = 5200u;
  i1 = 22u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 0u;
  f30(i0, i1);
  d0 = 0;
  goto B0;
  B25:;
  i0 = p0;
  i1 = p1;
  i2 = 48u;
  i1 = i1 == i2;
  if (i1) {
    i1 = l2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    p1 = i1;
    i2 = l4;
    i2 = i32_load(Z_envZ_memory, (u64)(i2));
    i1 = i1 < i2;
    if (i1) {
      i1 = l2;
      i2 = p1;
      i3 = 1u;
      i2 += i3;
      i32_store(Z_envZ_memory, (u64)(i1), i2);
      i1 = p1;
      i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
    } else {
      i1 = p0;
      i1 = f32_0(i1);
    }
    i2 = 32u;
    i1 |= i2;
    i2 = 120u;
    i1 = i1 == i2;
    if (i1) {
      i1 = p0;
      i2 = l6;
      i3 = l5;
      i4 = l7;
      d1 = f58(i1, i2, i3, i4);
      d0 = d1; goto B0;
    }
    i1 = l4;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    if (i1) {
      i1 = l2;
      i2 = l2;
      i2 = i32_load(Z_envZ_memory, (u64)(i2));
      i3 = 4294967295u;
      i2 += i3;
      i32_store(Z_envZ_memory, (u64)(i1), i2);
      i1 = 48u;
    } else {
      i1 = 48u;
    }
  } else {
    i1 = p1;
  }
  i2 = l6;
  i3 = l5;
  i4 = l7;
  d0 = f59(i0, i1, i2, i3, i4);
  goto B0;
  B24:;
  i0 = l4;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  if (i0) {
    i0 = l2;
    i1 = l2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 4294967295u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  }
  i0 = 5200u;
  i1 = 22u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 0u;
  f30(i0, i1);
  d0 = 0;
  goto B0;
  B13:;
  i0 = l4;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  if (i0) {
    i0 = l2;
    i1 = l2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 4294967295u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  }
  B12:;
  i0 = l7;
  f0 = (f32)(s32)(i0);
  d1 = g6;
  f1 = (f32)(d1);
  f0 *= f1;
  d0 = (f64)(f0);
  B0:;
  FUNC_EPILOGUE;
  return d0;
}

static f64 f58(u32 p0, u32 p1, u32 p2, u32 p3) {
  u32 l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0, l9 = 0, l10 = 0, l11 = 0, 
      l12 = 0, l13 = 0;
  u64 l14 = 0, l15 = 0, l16 = 0, l17 = 0;
  f64 l18 = 0, l19 = 0, l20 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5;
  u64 j0, j1, j2;
  f64 d0, d1, d2, d3, d4, d5;
  i0 = p0;
  i1 = 4u;
  i0 += i1;
  l6 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l4 = i0;
  i1 = p0;
  i2 = 100u;
  i1 += i2;
  l7 = i1;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i0 = i0 < i1;
  if (i0) {
    i0 = l6;
    i1 = l4;
    i2 = 1u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l4;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
  } else {
    i0 = p0;
    i0 = f32_0(i0);
  }
  l5 = i0;
  L3: 
    i0 = l5;
    i1 = 46u;
    i0 -= i1;
    switch (i0) {
      case 0: goto B2;
      case 1: goto B4;
      case 2: goto B5;
      default: goto B4;
    }
    B5:;
    i0 = l6;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l4 = i0;
    i1 = l7;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 < i1;
    if (i0) {
      i0 = l6;
      i1 = l4;
      i2 = 1u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l4;
      i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    } else {
      i0 = p0;
      i0 = f32_0(i0);
    }
    l5 = i0;
    i0 = 1u;
    l8 = i0;
    goto L3;
    B4:;
  goto B1;
  B2:;
  i0 = l6;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l4 = i0;
  i1 = l7;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i0 = i0 < i1;
  if (i0) {
    i0 = l6;
    i1 = l4;
    i2 = 1u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l4;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
  } else {
    i0 = p0;
    i0 = f32_0(i0);
  }
  l5 = i0;
  i1 = 48u;
  i0 = i0 == i1;
  if (i0) {
    L9: 
      j0 = l14;
      j1 = 18446744073709551615ull;
      j0 += j1;
      l14 = j0;
      i0 = l6;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l4 = i0;
      i1 = l7;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i0 = i0 < i1;
      if (i0) {
        i0 = l6;
        i1 = l4;
        i2 = 1u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l4;
        i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
      } else {
        i0 = p0;
        i0 = f32_0(i0);
      }
      l5 = i0;
      i1 = 48u;
      i0 = i0 == i1;
      if (i0) {goto L9;}
      j0 = l14;
      l16 = j0;
      i0 = 1u;
      l8 = i0;
      i0 = 1u;
  } else {
    i0 = 1u;
  }
  l9 = i0;
  B1:;
  j0 = 0ull;
  l14 = j0;
  d0 = 1;
  l19 = d0;
  i0 = 0u;
  l4 = i0;
  L11: 
    i0 = l5;
    i1 = 32u;
    i0 |= i1;
    l10 = i0;
    i0 = l5;
    i1 = 4294967248u;
    i0 += i1;
    l12 = i0;
    i1 = 10u;
    i0 = i0 < i1;
    if (i0) {goto B14;}
    i0 = l5;
    i1 = 46u;
    i0 = i0 == i1;
    l13 = i0;
    i1 = l10;
    i2 = 4294967199u;
    i1 += i2;
    i2 = 6u;
    i1 = i1 < i2;
    i0 |= i1;
    i0 = !(i0);
    if (i0) {goto B12;}
    i0 = l13;
    i0 = !(i0);
    if (i0) {goto B14;}
    i0 = l9;
    if (i0) {
      i0 = 46u;
      l5 = i0;
      goto B12;
    } else {
      j0 = l14;
      l15 = j0;
      j0 = l14;
      l16 = j0;
      i0 = 1u;
    }
    l9 = i0;
    goto B13;
    B14:;
    i0 = l10;
    i1 = 4294967209u;
    i0 += i1;
    i1 = l12;
    i2 = l5;
    i3 = 57u;
    i2 = (u32)((s32)i2 > (s32)i3);
    i0 = i2 ? i0 : i1;
    l5 = i0;
    j0 = l14;
    j1 = 8ull;
    i0 = (u64)((s64)j0 < (s64)j1);
    if (i0) {
      d0 = l19;
      l20 = d0;
      i0 = l5;
      i1 = l4;
      i2 = 4u;
      i1 <<= (i2 & 31);
      i0 += i1;
      l4 = i0;
    } else {
      j0 = l14;
      j1 = 14ull;
      i0 = (u64)((s64)j0 < (s64)j1);
      if (i0) {
        d0 = l19;
        d1 = 0.0625;
        d0 *= d1;
        l19 = d0;
        l20 = d0;
        d0 = l18;
        d1 = l19;
        i2 = l5;
        d2 = (f64)(s32)(i2);
        d1 *= d2;
        d0 += d1;
      } else {
        i0 = l11;
        i1 = 1u;
        i2 = l5;
        i2 = !(i2);
        i3 = l11;
        i4 = 0u;
        i3 = i3 != i4;
        i2 |= i3;
        l5 = i2;
        i0 = i2 ? i0 : i1;
        l11 = i0;
        d0 = l19;
        l20 = d0;
        d0 = l18;
        d1 = l18;
        d2 = l19;
        d3 = 0.5;
        d2 *= d3;
        d1 += d2;
        i2 = l5;
        d0 = i2 ? d0 : d1;
      }
      l18 = d0;
    }
    j0 = l14;
    j1 = 1ull;
    j0 += j1;
    l15 = j0;
    d0 = l20;
    l19 = d0;
    i0 = 1u;
    l8 = i0;
    B13:;
    i0 = l6;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l5 = i0;
    i1 = l7;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 < i1;
    if (i0) {
      i0 = l6;
      i1 = l5;
      i2 = 1u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l5;
      i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    } else {
      i0 = p0;
      i0 = f32_0(i0);
    }
    l5 = i0;
    j0 = l15;
    l14 = j0;
    goto L11;
    B12:;
  i0 = l8;
  if (i0) {
    j0 = l14;
    j1 = 8ull;
    i0 = (u64)((s64)j0 < (s64)j1);
    if (i0) {
      j0 = l14;
      l15 = j0;
      L22: 
        i0 = l4;
        i1 = 4u;
        i0 <<= (i1 & 31);
        l4 = i0;
        j0 = l15;
        j1 = 1ull;
        j0 += j1;
        l17 = j0;
        j0 = l15;
        j1 = 7ull;
        i0 = (u64)((s64)j0 < (s64)j1);
        if (i0) {
          j0 = l17;
          l15 = j0;
          goto L22;
        }
    }
    i0 = l5;
    i1 = 32u;
    i0 |= i1;
    i1 = 112u;
    i0 = i0 == i1;
    if (i0) {
      i0 = p0;
      j0 = f60(i0);
      l15 = j0;
      j1 = 9223372036854775808ull;
      i0 = j0 == j1;
      if (i0) {
        i0 = p0;
        i1 = 0u;
        f30(i0, i1);
        d0 = 0;
        goto B20;
      }
    } else {
      i0 = l7;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      if (i0) {
        i0 = l6;
        i1 = l6;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i2 = 4294967295u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        j0 = 0ull;
      } else {
        j0 = 0ull;
      }
      l15 = j0;
    }
    i0 = p3;
    d0 = (f64)(s32)(i0);
    d1 = 0;
    d0 *= d1;
    i1 = l4;
    i1 = !(i1);
    if (i1) {goto B20;}
    j0 = l15;
    j1 = l16;
    j2 = l14;
    i3 = l9;
    j1 = i3 ? j1 : j2;
    j2 = 2ull;
    j1 <<= (j2 & 63);
    j2 = 18446744073709551584ull;
    j1 += j2;
    j0 += j1;
    l14 = j0;
    i1 = 0u;
    i2 = p2;
    i1 -= i2;
    j1 = (u64)(s64)(s32)(i1);
    i0 = (u64)((s64)j0 > (s64)j1);
    if (i0) {
      i0 = 5200u;
      i1 = 34u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p3;
      d0 = (f64)(s32)(i0);
      d1 = 1.7976931348623157e+308;
      d0 *= d1;
      d1 = 1.7976931348623157e+308;
      d0 *= d1;
      goto B20;
    }
    j0 = l14;
    i1 = p2;
    i2 = 4294967190u;
    i1 += i2;
    j1 = (u64)(s64)(s32)(i1);
    i0 = (u64)((s64)j0 < (s64)j1);
    if (i0) {
      i0 = 5200u;
      i1 = 34u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p3;
      d0 = (f64)(s32)(i0);
      d1 = 2.2250738585072014e-308;
      d0 *= d1;
      d1 = 2.2250738585072014e-308;
      d0 *= d1;
      goto B20;
    }
    i0 = l4;
    i1 = 4294967295u;
    i0 = (u32)((s32)i0 > (s32)i1);
    if (i0) {
      i0 = l4;
      p0 = i0;
      L30: 
        d0 = l18;
        d1 = 0.5;
        i0 = d0 >= d1;
        i0 = !(i0);
        l4 = i0;
        i1 = 1u;
        i0 ^= i1;
        i1 = p0;
        i2 = 1u;
        i1 <<= (i2 & 31);
        i0 |= i1;
        p0 = i0;
        d0 = l18;
        d1 = l18;
        d2 = l18;
        d3 = -1;
        d2 += d3;
        i3 = l4;
        d1 = i3 ? d1 : d2;
        d0 += d1;
        l18 = d0;
        j0 = l14;
        j1 = 18446744073709551615ull;
        j0 += j1;
        l14 = j0;
        i0 = p0;
        i1 = 4294967295u;
        i0 = (u32)((s32)i0 > (s32)i1);
        if (i0) {goto L30;}
    } else {
      i0 = l4;
      p0 = i0;
    }
    j0 = l14;
    j1 = 32ull;
    i2 = p2;
    j2 = (u64)(s64)(s32)(i2);
    j1 -= j2;
    j0 += j1;
    l16 = j0;
    i1 = p1;
    j1 = (u64)(s64)(s32)(i1);
    i0 = (u64)((s64)j0 < (s64)j1);
    if (i0) {
      j0 = l16;
      i0 = (u32)(j0);
      p1 = i0;
      i1 = 0u;
      i0 = (u32)((s32)i0 <= (s32)i1);
      if (i0) {
        i0 = 0u;
        p1 = i0;
        i0 = 84u;
        p2 = i0;
        goto B32;
      }
    }
    i0 = 84u;
    i1 = p1;
    i0 -= i1;
    p2 = i0;
    i0 = p1;
    i1 = 53u;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {goto B32;}
    i0 = p3;
    d0 = (f64)(s32)(i0);
    l19 = d0;
    d0 = 0;
    goto B31;
    B32:;
    d0 = 1;
    i1 = p2;
    d0 = f61(d0, i1);
    i1 = p3;
    d1 = (f64)(s32)(i1);
    l19 = d1;
    d0 = f62(d0, d1);
    B31:;
    l20 = d0;
    d0 = 0;
    d1 = l18;
    i2 = p0;
    i3 = 1u;
    i2 &= i3;
    i2 = !(i2);
    i3 = p1;
    i4 = 32u;
    i3 = (u32)((s32)i3 < (s32)i4);
    d4 = l18;
    d5 = 0;
    i4 = d4 != d5;
    i3 &= i4;
    i2 &= i3;
    p1 = i2;
    d0 = i2 ? d0 : d1;
    d1 = l19;
    d0 *= d1;
    d1 = l20;
    d2 = l19;
    i3 = p0;
    i4 = p1;
    i5 = 1u;
    i4 &= i5;
    i3 += i4;
    d3 = (f64)(i3);
    d2 *= d3;
    d1 += d2;
    d0 += d1;
    d1 = l20;
    d0 -= d1;
    l18 = d0;
    d1 = 0;
    i0 = d0 == d1;
    if (i0) {
      i0 = 5200u;
      i1 = 34u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    d0 = l18;
    j1 = l14;
    i1 = (u32)(j1);
    d0 = f64_0(d0, i1);
    B20:;
  } else {
    i0 = l7;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    if (i0) {
      i0 = l6;
      i1 = l6;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 4294967295u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    i0 = p0;
    i1 = 0u;
    f30(i0, i1);
    i0 = p3;
    d0 = (f64)(s32)(i0);
    d1 = 0;
    d0 *= d1;
  }
  FUNC_EPILOGUE;
  return d0;
}

static f64 f59(u32 p0, u32 p1, u32 p2, u32 p3, u32 p4) {
  u32 l5 = 0, l6 = 0, l7 = 0, l8 = 0, l9 = 0, l10 = 0, l11 = 0, l12 = 0, 
      l13 = 0, l14 = 0, l15 = 0, l16 = 0, l17 = 0, l18 = 0, l19 = 0;
  u64 l20 = 0, l21 = 0, l22 = 0;
  f64 l23 = 0, l24 = 0, l25 = 0, l26 = 0, l27 = 0, l28 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5;
  u64 j0, j1, j2;
  f64 d0, d1, d2, d3;
  i0 = g7;
  l17 = i0;
  i0 = g7;
  i1 = 512u;
  i0 += i1;
  g7 = i0;
  i0 = l17;
  l11 = i0;
  i0 = 0u;
  i1 = p2;
  i2 = p3;
  i1 += i2;
  l18 = i1;
  i0 -= i1;
  l19 = i0;
  i0 = p0;
  i1 = 4u;
  i0 += i1;
  l7 = i0;
  i0 = p0;
  i1 = 100u;
  i0 += i1;
  l14 = i0;
  L2: 
    i0 = p1;
    i1 = 46u;
    i0 -= i1;
    switch (i0) {
      case 0: goto B1;
      case 1: goto B5;
      case 2: goto B4;
      default: goto B5;
    }
    B5:;
    i0 = p1;
    l9 = i0;
    i0 = l5;
    p1 = i0;
    goto B3;
    B4:;
    i0 = l7;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    p1 = i0;
    i1 = l14;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i0 = i0 < i1;
    if (i0) {
      i0 = l7;
      i1 = p1;
      i2 = 1u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p1;
      i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    } else {
      i0 = p0;
      i0 = f32_0(i0);
    }
    p1 = i0;
    i0 = 1u;
    l5 = i0;
    goto L2;
    B3:;
  goto B0;
  B1:;
  i0 = l7;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  p1 = i0;
  i1 = l14;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i0 = i0 < i1;
  if (i0) {
    i0 = l7;
    i1 = p1;
    i2 = 1u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
  } else {
    i0 = p0;
    i0 = f32_0(i0);
  }
  l9 = i0;
  i1 = 48u;
  i0 = i0 == i1;
  if (i0) {
    L9: 
      j0 = l21;
      j1 = 18446744073709551615ull;
      j0 += j1;
      l21 = j0;
      i0 = l7;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      p1 = i0;
      i1 = l14;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i0 = i0 < i1;
      if (i0) {
        i0 = l7;
        i1 = p1;
        i2 = 1u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p1;
        i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
      } else {
        i0 = p0;
        i0 = f32_0(i0);
      }
      l9 = i0;
      i1 = 48u;
      i0 = i0 == i1;
      if (i0) {goto L9;}
      i0 = 1u;
      l6 = i0;
      i0 = 1u;
  } else {
    i0 = 1u;
    l6 = i0;
    i0 = l5;
  }
  p1 = i0;
  B0:;
  i0 = l11;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l9;
  i1 = 46u;
  i0 = i0 == i1;
  l12 = i0;
  i1 = l9;
  i2 = 4294967248u;
  i1 += i2;
  l15 = i1;
  i2 = 10u;
  i1 = i1 < i2;
  i0 |= i1;
  if (i0) {
    i0 = l11;
    i1 = 496u;
    i0 += i1;
    l16 = i0;
    i0 = 0u;
    l5 = i0;
    i0 = l9;
    l13 = i0;
    i0 = l15;
    l9 = i0;
    L18: 
      i0 = l12;
      if (i0) {
        i0 = l6;
        if (i0) {goto B19;}
        i0 = 1u;
        l6 = i0;
        j0 = l20;
        l21 = j0;
        j0 = l20;
      } else {
        j0 = l20;
        j1 = 1ull;
        j0 += j1;
        l20 = j0;
        i0 = l13;
        i1 = 48u;
        i0 = i0 != i1;
        l12 = i0;
        i0 = l8;
        i1 = 125u;
        i0 = (u32)((s32)i0 >= (s32)i1);
        if (i0) {
          j0 = l20;
          i1 = l12;
          i1 = !(i1);
          if (i1) {goto B21;}
          i0 = l16;
          i1 = l16;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          i2 = 1u;
          i1 |= i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          j0 = l20;
          goto B21;
        }
        i0 = l8;
        i1 = 2u;
        i0 <<= (i1 & 31);
        i1 = l11;
        i0 += i1;
        p1 = i0;
        i1 = l10;
        if (i1) {
          i1 = l13;
          i2 = 4294967248u;
          i1 += i2;
          i2 = p1;
          i2 = i32_load(Z_envZ_memory, (u64)(i2));
          i3 = 10u;
          i2 *= i3;
          i1 += i2;
        } else {
          i1 = l9;
        }
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l10;
        i1 = 1u;
        i0 += i1;
        p1 = i0;
        i1 = 9u;
        i0 = i0 == i1;
        l9 = i0;
        i0 = 0u;
        i1 = p1;
        i2 = l9;
        i0 = i2 ? i0 : i1;
        l10 = i0;
        i0 = l8;
        i1 = l9;
        i0 += i1;
        l8 = i0;
        j0 = l20;
        i0 = (u32)(j0);
        i1 = l5;
        i2 = l12;
        i0 = i2 ? i0 : i1;
        l5 = i0;
        i0 = 1u;
        p1 = i0;
        j0 = l20;
        B21:;
      }
      l22 = j0;
      i0 = l7;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l9 = i0;
      i1 = l14;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i0 = i0 < i1;
      if (i0) {
        i0 = l7;
        i1 = l9;
        i2 = 1u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l9;
        i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
      } else {
        i0 = p0;
        i0 = f32_0(i0);
      }
      l13 = i0;
      i1 = 4294967248u;
      i0 += i1;
      l9 = i0;
      i1 = 10u;
      i0 = i0 < i1;
      i1 = l13;
      i2 = 46u;
      i1 = i1 == i2;
      l12 = i1;
      i0 |= i1;
      if (i0) {
        j0 = l22;
        l20 = j0;
        goto L18;
      } else {
        i0 = l13;
        l9 = i0;
        goto B17;
      }
      UNREACHABLE;
      B19:;
    i0 = p1;
    i1 = 0u;
    i0 = i0 != i1;
    p1 = i0;
    goto B15;
    B17:;
  } else {
    i0 = 0u;
    l5 = i0;
  }
  j0 = l21;
  j1 = l22;
  i2 = l6;
  j0 = i2 ? j0 : j1;
  l21 = j0;
  i0 = p1;
  i1 = 0u;
  i0 = i0 != i1;
  p1 = i0;
  i1 = l9;
  i2 = 32u;
  i1 |= i2;
  i2 = 101u;
  i1 = i1 == i2;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {
    i0 = l9;
    i1 = 4294967295u;
    i0 = (u32)((s32)i0 > (s32)i1);
    if (i0) {
      j0 = l22;
      l20 = j0;
      goto B15;
    } else {
      goto B14;
    }
    UNREACHABLE;
  }
  i0 = p0;
  j0 = f60(i0);
  l20 = j0;
  j1 = 9223372036854775808ull;
  i0 = j0 == j1;
  if (i0) {
    i0 = p0;
    i1 = 0u;
    f30(i0, i1);
    d0 = 0;
    goto B11;
  }
  j0 = l20;
  j1 = l21;
  j0 += j1;
  l21 = j0;
  goto B12;
  B15:;
  i0 = l14;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  if (i0) {
    i0 = l7;
    i1 = l7;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 4294967295u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    i0 = !(i0);
    if (i0) {goto B13;}
    j0 = l20;
    l22 = j0;
    goto B12;
  } else {
    j0 = l20;
  }
  l22 = j0;
  B14:;
  i0 = p1;
  i0 = !(i0);
  if (i0) {goto B13;}
  goto B12;
  B13:;
  i0 = 5200u;
  i1 = 22u;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = 0u;
  f30(i0, i1);
  d0 = 0;
  goto B11;
  B12:;
  i0 = p4;
  d0 = (f64)(s32)(i0);
  d1 = 0;
  d0 *= d1;
  i1 = l11;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  p0 = i1;
  i1 = !(i1);
  if (i1) {goto B11;}
  j0 = l21;
  j1 = l22;
  i0 = j0 == j1;
  j1 = l22;
  j2 = 10ull;
  i1 = (u64)((s64)j1 < (s64)j2);
  i0 &= i1;
  if (i0) {
    i0 = p4;
    d0 = (f64)(s32)(i0);
    i1 = p0;
    d1 = (f64)(i1);
    d0 *= d1;
    i1 = p0;
    i2 = p2;
    i1 >>= (i2 & 31);
    i1 = !(i1);
    i2 = p2;
    i3 = 30u;
    i2 = (u32)((s32)i2 > (s32)i3);
    i1 |= i2;
    if (i1) {goto B11;}
  }
  j0 = l21;
  i1 = p3;
  i2 = 4294967294u;
  i1 = I32_DIV_S(i1, i2);
  j1 = (u64)(s64)(s32)(i1);
  i0 = (u64)((s64)j0 > (s64)j1);
  if (i0) {
    i0 = 5200u;
    i1 = 34u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p4;
    d0 = (f64)(s32)(i0);
    d1 = 1.7976931348623157e+308;
    d0 *= d1;
    d1 = 1.7976931348623157e+308;
    d0 *= d1;
    goto B11;
  }
  j0 = l21;
  i1 = p3;
  i2 = 4294967190u;
  i1 += i2;
  j1 = (u64)(s64)(s32)(i1);
  i0 = (u64)((s64)j0 < (s64)j1);
  if (i0) {
    i0 = 5200u;
    i1 = 34u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p4;
    d0 = (f64)(s32)(i0);
    d1 = 2.2250738585072014e-308;
    d0 *= d1;
    d1 = 2.2250738585072014e-308;
    d0 *= d1;
    goto B11;
  }
  i0 = l10;
  if (i0) {
    i0 = l10;
    i1 = 9u;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {
      i0 = l8;
      i1 = 2u;
      i0 <<= (i1 & 31);
      i1 = l11;
      i0 += i1;
      l7 = i0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      p1 = i0;
      L35: 
        i0 = p1;
        i1 = 10u;
        i0 *= i1;
        p1 = i0;
        i0 = l10;
        i1 = 1u;
        i0 += i1;
        p0 = i0;
        i0 = l10;
        i1 = 8u;
        i0 = (u32)((s32)i0 < (s32)i1);
        if (i0) {
          i0 = p0;
          l10 = i0;
          goto L35;
        }
      i0 = l7;
      i1 = p1;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
    }
    i0 = l8;
    i1 = 1u;
    i0 += i1;
    l8 = i0;
  }
  j0 = l21;
  i0 = (u32)(j0);
  p1 = i0;
  i0 = l5;
  i1 = 9u;
  i0 = (u32)((s32)i0 < (s32)i1);
  if (i0) {
    i0 = p1;
    i1 = 18u;
    i0 = (u32)((s32)i0 < (s32)i1);
    i1 = l5;
    i2 = p1;
    i1 = (u32)((s32)i1 <= (s32)i2);
    i0 &= i1;
    if (i0) {
      i0 = p1;
      i1 = 9u;
      i0 = i0 == i1;
      if (i0) {
        i0 = p4;
        d0 = (f64)(s32)(i0);
        i1 = l11;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        d1 = (f64)(i1);
        d0 *= d1;
        goto B11;
      }
      i0 = p1;
      i1 = 9u;
      i0 = (u32)((s32)i0 < (s32)i1);
      if (i0) {
        i0 = p4;
        d0 = (f64)(s32)(i0);
        i1 = l11;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        d1 = (f64)(i1);
        d0 *= d1;
        i1 = 0u;
        i2 = p1;
        i1 -= i2;
        i2 = 2u;
        i1 <<= (i2 & 31);
        i2 = 2016u;
        i1 += i2;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        d1 = (f64)(s32)(i1);
        d0 /= d1;
        goto B11;
      }
      i0 = p2;
      i1 = 27u;
      i0 += i1;
      i1 = p1;
      i2 = 4294967293u;
      i1 *= i2;
      i0 += i1;
      l7 = i0;
      i1 = 30u;
      i0 = (u32)((s32)i0 > (s32)i1);
      i1 = l11;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      p0 = i1;
      i2 = l7;
      i1 >>= (i2 & 31);
      i1 = !(i1);
      i0 |= i1;
      if (i0) {
        i0 = p4;
        d0 = (f64)(s32)(i0);
        i1 = p0;
        d1 = (f64)(i1);
        d0 *= d1;
        i1 = p1;
        i2 = 2u;
        i1 <<= (i2 & 31);
        i2 = 1944u;
        i1 += i2;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        d1 = (f64)(s32)(i1);
        d0 *= d1;
        goto B11;
      }
    }
  }
  i0 = p1;
  i1 = 9u;
  i0 = I32_REM_S(i0, i1);
  p0 = i0;
  if (i0) {
    i0 = 0u;
    i1 = p0;
    i2 = p0;
    i3 = 9u;
    i2 += i3;
    i3 = p1;
    i4 = 4294967295u;
    i3 = (u32)((s32)i3 > (s32)i4);
    i1 = i3 ? i1 : i2;
    l12 = i1;
    i0 -= i1;
    i1 = 2u;
    i0 <<= (i1 & 31);
    i1 = 2016u;
    i0 += i1;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l15 = i0;
    i0 = l8;
    if (i0) {
      i0 = 1000000000u;
      i1 = l15;
      i0 = I32_DIV_S(i0, i1);
      l9 = i0;
      i0 = 0u;
      l5 = i0;
      i0 = 0u;
      p0 = i0;
      i0 = 0u;
      l7 = i0;
      L44: 
        i0 = l5;
        i1 = l7;
        i2 = 2u;
        i1 <<= (i2 & 31);
        i2 = l11;
        i1 += i2;
        l10 = i1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        l6 = i1;
        i2 = l15;
        i1 = DIV_U(i1, i2);
        l5 = i1;
        i0 += i1;
        l13 = i0;
        i0 = l10;
        i1 = l13;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l9;
        i1 = l6;
        i2 = l5;
        i3 = l15;
        i2 *= i3;
        i1 -= i2;
        i0 *= i1;
        l5 = i0;
        i0 = p1;
        i1 = 4294967287u;
        i0 += i1;
        i1 = p1;
        i2 = l13;
        i2 = !(i2);
        i3 = p0;
        i4 = l7;
        i3 = i3 == i4;
        i2 &= i3;
        l6 = i2;
        i0 = i2 ? i0 : i1;
        p1 = i0;
        i0 = p0;
        i1 = 1u;
        i0 += i1;
        i1 = 127u;
        i0 &= i1;
        i1 = p0;
        i2 = l6;
        i0 = i2 ? i0 : i1;
        p0 = i0;
        i0 = l7;
        i1 = 1u;
        i0 += i1;
        l7 = i0;
        i1 = l8;
        i0 = i0 != i1;
        if (i0) {goto L44;}
      i0 = l5;
      if (i0) {
        i0 = l8;
        i1 = 2u;
        i0 <<= (i1 & 31);
        i1 = l11;
        i0 += i1;
        i1 = l5;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p0;
        l7 = i0;
        i0 = l8;
        i1 = 1u;
        i0 += i1;
      } else {
        i0 = p0;
        l7 = i0;
        i0 = l8;
      }
    } else {
      i0 = 0u;
      l7 = i0;
      i0 = 0u;
    }
    p0 = i0;
    i0 = p1;
    i1 = 9u;
    i2 = l12;
    i1 -= i2;
    i0 += i1;
    p1 = i0;
    i0 = l7;
  } else {
    i0 = l8;
    p0 = i0;
    i0 = 0u;
  }
  l5 = i0;
  i0 = 0u;
  l7 = i0;
  L46: 
    i0 = p1;
    i1 = 18u;
    i0 = (u32)((s32)i0 < (s32)i1);
    l15 = i0;
    i0 = p1;
    i1 = 18u;
    i0 = i0 == i1;
    l13 = i0;
    i0 = l5;
    i1 = 2u;
    i0 <<= (i1 & 31);
    i1 = l11;
    i0 += i1;
    l12 = i0;
    L48: 
      i0 = l15;
      i0 = !(i0);
      if (i0) {
        i0 = l13;
        i0 = !(i0);
        if (i0) {goto B47;}
        i0 = l12;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        i1 = 9007199u;
        i0 = i0 >= i1;
        if (i0) {
          i0 = 18u;
          p1 = i0;
          goto B47;
        }
      }
      i0 = 0u;
      l8 = i0;
      i0 = p0;
      i1 = 127u;
      i0 += i1;
      l6 = i0;
      L51: 
        i0 = l8;
        j0 = (u64)(i0);
        i1 = l6;
        i2 = 127u;
        i1 &= i2;
        l16 = i1;
        i2 = 2u;
        i1 <<= (i2 & 31);
        i2 = l11;
        i1 += i2;
        l10 = i1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        j1 = (u64)(i1);
        j2 = 29ull;
        j1 <<= (j2 & 63);
        j0 += j1;
        l22 = j0;
        i0 = (u32)(j0);
        l6 = i0;
        j0 = l22;
        j1 = 1000000000ull;
        i0 = j0 > j1;
        if (i0) {
          j0 = l22;
          j1 = 1000000000ull;
          j0 = DIV_U(j0, j1);
          l21 = j0;
          i0 = (u32)(j0);
          l8 = i0;
          j0 = l22;
          j1 = l21;
          j2 = 1000000000ull;
          j1 *= j2;
          j0 -= j1;
          i0 = (u32)(j0);
          l6 = i0;
        } else {
          i0 = 0u;
          l8 = i0;
        }
        i0 = l10;
        i1 = l6;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p0;
        i1 = p0;
        i2 = l16;
        i3 = l6;
        i1 = i3 ? i1 : i2;
        i2 = l5;
        i3 = l16;
        i2 = i2 == i3;
        l9 = i2;
        i3 = l16;
        i4 = p0;
        i5 = 127u;
        i4 += i5;
        i5 = 127u;
        i4 &= i5;
        i3 = i3 != i4;
        i2 |= i3;
        i0 = i2 ? i0 : i1;
        l10 = i0;
        i0 = l16;
        i1 = 4294967295u;
        i0 += i1;
        l6 = i0;
        i0 = l9;
        i0 = !(i0);
        if (i0) {
          i0 = l10;
          p0 = i0;
          goto L51;
        }
      i0 = l7;
      i1 = 4294967267u;
      i0 += i1;
      l7 = i0;
      i0 = l8;
      i0 = !(i0);
      if (i0) {goto L48;}
    i0 = l10;
    i1 = 127u;
    i0 += i1;
    i1 = 127u;
    i0 &= i1;
    l6 = i0;
    i0 = l10;
    i1 = 126u;
    i0 += i1;
    i1 = 127u;
    i0 &= i1;
    i1 = 2u;
    i0 <<= (i1 & 31);
    i1 = l11;
    i0 += i1;
    l9 = i0;
    i0 = l5;
    i1 = 127u;
    i0 += i1;
    i1 = 127u;
    i0 &= i1;
    l5 = i0;
    i1 = l10;
    i0 = i0 == i1;
    if (i0) {
      i0 = l9;
      i1 = l6;
      i2 = 2u;
      i1 <<= (i2 & 31);
      i2 = l11;
      i1 += i2;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = l9;
      i2 = i32_load(Z_envZ_memory, (u64)(i2));
      i1 |= i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l6;
      p0 = i0;
    }
    i0 = l5;
    i1 = 2u;
    i0 <<= (i1 & 31);
    i1 = l11;
    i0 += i1;
    i1 = l8;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    i1 = 9u;
    i0 += i1;
    p1 = i0;
    goto L46;
    B47:;
  L55: 
    i0 = p0;
    i1 = 1u;
    i0 += i1;
    i1 = 127u;
    i0 &= i1;
    l9 = i0;
    i0 = p0;
    i1 = 127u;
    i0 += i1;
    i1 = 127u;
    i0 &= i1;
    i1 = 2u;
    i0 <<= (i1 & 31);
    i1 = l11;
    i0 += i1;
    l16 = i0;
    i0 = p1;
    l6 = i0;
    L57: 
      i0 = l6;
      i1 = 18u;
      i0 = i0 == i1;
      l10 = i0;
      i0 = 9u;
      i1 = 1u;
      i2 = l6;
      i3 = 27u;
      i2 = (u32)((s32)i2 > (s32)i3);
      i0 = i2 ? i0 : i1;
      l14 = i0;
      i0 = l5;
      p1 = i0;
      L59: 
        i0 = 0u;
        l12 = i0;
        L62: 
          i0 = p0;
          i1 = p1;
          i2 = l12;
          i1 += i2;
          i2 = 127u;
          i1 &= i2;
          l5 = i1;
          i0 = i0 == i1;
          if (i0) {goto B61;}
          i0 = l5;
          i1 = 2u;
          i0 <<= (i1 & 31);
          i1 = l11;
          i0 += i1;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l8 = i0;
          i1 = l12;
          i2 = 2u;
          i1 <<= (i2 & 31);
          i2 = 2516u;
          i1 += i2;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          l5 = i1;
          i0 = i0 < i1;
          if (i0) {goto B61;}
          i0 = l8;
          i1 = l5;
          i0 = i0 > i1;
          if (i0) {goto B63;}
          i0 = l12;
          i1 = 1u;
          i0 += i1;
          i1 = 2u;
          i0 = i0 >= i1;
          if (i0) {goto B61;}
          i0 = 1u;
          l12 = i0;
          goto L62;
          B63:;
        goto B60;
        B61:;
        i0 = l10;
        if (i0) {goto B56;}
        B60:;
        i0 = l7;
        i1 = l14;
        i0 += i1;
        l7 = i0;
        i0 = p0;
        i1 = p1;
        i0 = i0 == i1;
        if (i0) {
          i0 = p0;
          p1 = i0;
          goto L59;
        }
      i0 = 1u;
      i1 = l14;
      i0 <<= (i1 & 31);
      i1 = 4294967295u;
      i0 += i1;
      l13 = i0;
      i0 = 1000000000u;
      i1 = l14;
      i0 >>= (i1 & 31);
      l12 = i0;
      i0 = 0u;
      l10 = i0;
      i0 = p1;
      l5 = i0;
      i0 = p1;
      l8 = i0;
      L65: 
        i0 = l10;
        i1 = l8;
        i2 = 2u;
        i1 <<= (i2 & 31);
        i2 = l11;
        i1 += i2;
        l10 = i1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        p1 = i1;
        i2 = l14;
        i1 >>= (i2 & 31);
        i0 += i1;
        l15 = i0;
        i0 = l10;
        i1 = l15;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l12;
        i1 = p1;
        i2 = l13;
        i1 &= i2;
        i0 *= i1;
        l10 = i0;
        i0 = l6;
        i1 = 4294967287u;
        i0 += i1;
        i1 = l6;
        i2 = l15;
        i2 = !(i2);
        i3 = l5;
        i4 = l8;
        i3 = i3 == i4;
        i2 &= i3;
        l6 = i2;
        i0 = i2 ? i0 : i1;
        p1 = i0;
        i0 = l5;
        i1 = 1u;
        i0 += i1;
        i1 = 127u;
        i0 &= i1;
        i1 = l5;
        i2 = l6;
        i0 = i2 ? i0 : i1;
        l5 = i0;
        i0 = l8;
        i1 = 1u;
        i0 += i1;
        i1 = 127u;
        i0 &= i1;
        l8 = i0;
        i1 = p0;
        i0 = i0 != i1;
        if (i0) {
          i0 = p1;
          l6 = i0;
          goto L65;
        }
      i0 = l10;
      if (i0) {
        i0 = l5;
        i1 = l9;
        i0 = i0 != i1;
        if (i0) {goto B58;}
        i0 = l16;
        i1 = l16;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i2 = 1u;
        i1 |= i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
      }
      i0 = p1;
      l6 = i0;
      goto L57;
      B58:;
    i0 = p0;
    i1 = 2u;
    i0 <<= (i1 & 31);
    i1 = l11;
    i0 += i1;
    i1 = l10;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l9;
    p0 = i0;
    goto L55;
    B56:;
  i0 = 0u;
  l5 = i0;
  L68: 
    i0 = p0;
    i1 = 1u;
    i0 += i1;
    i1 = 127u;
    i0 &= i1;
    l6 = i0;
    i0 = p0;
    i1 = p1;
    i2 = l5;
    i1 += i2;
    i2 = 127u;
    i1 &= i2;
    l8 = i1;
    i0 = i0 == i1;
    if (i0) {
      i0 = l6;
      i1 = 4294967295u;
      i0 += i1;
      i1 = 2u;
      i0 <<= (i1 & 31);
      i1 = l11;
      i0 += i1;
      i1 = 0u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l6;
      p0 = i0;
    }
    d0 = l23;
    d1 = 1000000000;
    d0 *= d1;
    i1 = l8;
    i2 = 2u;
    i1 <<= (i2 & 31);
    i2 = l11;
    i1 += i2;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    d1 = (f64)(i1);
    d0 += d1;
    l23 = d0;
    i0 = l5;
    i1 = 1u;
    i0 += i1;
    l5 = i0;
    i1 = 2u;
    i0 = i0 != i1;
    if (i0) {goto L68;}
  d0 = l23;
  i1 = p4;
  d1 = (f64)(s32)(i1);
  l25 = d1;
  d0 *= d1;
  l24 = d0;
  i0 = l7;
  i1 = 53u;
  i0 += i1;
  p4 = i0;
  i1 = p3;
  i0 -= i1;
  l5 = i0;
  i1 = p2;
  i0 = (u32)((s32)i0 < (s32)i1);
  p3 = i0;
  i0 = l5;
  i1 = 0u;
  i2 = l5;
  i3 = 0u;
  i2 = (u32)((s32)i2 > (s32)i3);
  i0 = i2 ? i0 : i1;
  i1 = p2;
  i2 = p3;
  i0 = i2 ? i0 : i1;
  l6 = i0;
  i1 = 53u;
  i0 = (u32)((s32)i0 < (s32)i1);
  if (i0) {
    d0 = 1;
    i1 = 105u;
    i2 = l6;
    i1 -= i2;
    d0 = f61(d0, i1);
    d1 = l24;
    d0 = f62(d0, d1);
    l26 = d0;
    l27 = d0;
    d0 = l24;
    d1 = 1;
    i2 = 53u;
    i3 = l6;
    i2 -= i3;
    d1 = f61(d1, i2);
    d0 = f63(d0, d1);
    l28 = d0;
    l23 = d0;
    d0 = l26;
    d1 = l24;
    d2 = l28;
    d1 -= d2;
    d0 += d1;
    l24 = d0;
  } else {
    d0 = 0;
    l23 = d0;
  }
  i0 = p1;
  i1 = 2u;
  i0 += i1;
  i1 = 127u;
  i0 &= i1;
  p2 = i0;
  i1 = p0;
  i0 = i0 != i1;
  if (i0) {
    i0 = p2;
    i1 = 2u;
    i0 <<= (i1 & 31);
    i1 = l11;
    i0 += i1;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    p2 = i0;
    i1 = 500000000u;
    i0 = i0 < i1;
    if (i0) {
      i0 = p2;
      i0 = !(i0);
      if (i0) {
        i0 = p0;
        i1 = p1;
        i2 = 3u;
        i1 += i2;
        i2 = 127u;
        i1 &= i2;
        i0 = i0 == i1;
        if (i0) {goto B72;}
      }
      d0 = l25;
      d1 = 0.25;
      d0 *= d1;
      d1 = l23;
      d0 += d1;
    } else {
      i0 = p2;
      i1 = 500000000u;
      i0 = i0 != i1;
      if (i0) {
        d0 = l25;
        d1 = 0.75;
        d0 *= d1;
        d1 = l23;
        d0 += d1;
        l23 = d0;
        goto B72;
      }
      i0 = p0;
      i1 = p1;
      i2 = 3u;
      i1 += i2;
      i2 = 127u;
      i1 &= i2;
      i0 = i0 == i1;
      if (i0) {
        d0 = l25;
        d1 = 0.5;
        d0 *= d1;
        d1 = l23;
        d0 += d1;
      } else {
        d0 = l25;
        d1 = 0.75;
        d0 *= d1;
        d1 = l23;
        d0 += d1;
      }
    }
    l23 = d0;
    B72:;
    i0 = 53u;
    i1 = l6;
    i0 -= i1;
    i1 = 1u;
    i0 = (u32)((s32)i0 > (s32)i1);
    if (i0) {
      d0 = l23;
      d1 = 1;
      d0 = f63(d0, d1);
      d1 = 0;
      i0 = d0 == d1;
      if (i0) {
        d0 = l23;
        d1 = 1;
        d0 += d1;
      } else {
        d0 = l23;
      }
    } else {
      d0 = l23;
    }
    l23 = d0;
  }
  d0 = l24;
  d1 = l23;
  d0 += d1;
  d1 = l27;
  d0 -= d1;
  l24 = d0;
  i0 = p4;
  i1 = 2147483647u;
  i0 &= i1;
  i1 = 4294967294u;
  i2 = l18;
  i1 -= i2;
  i0 = (u32)((s32)i0 > (s32)i1);
  if (i0) {
    i0 = l7;
    d1 = l24;
    d1 = fabs(d1);
    d2 = 9007199254740992;
    i1 = d1 >= d2;
    i1 = !(i1);
    p0 = i1;
    i2 = 1u;
    i1 ^= i2;
    i0 += i1;
    l7 = i0;
    d0 = l24;
    d1 = l24;
    d2 = 0.5;
    d1 *= d2;
    i2 = p0;
    d0 = i2 ? d0 : d1;
    l24 = d0;
    i0 = l7;
    i1 = 50u;
    i0 += i1;
    i1 = l19;
    i0 = (u32)((s32)i0 <= (s32)i1);
    if (i0) {
      d0 = l24;
      i1 = p3;
      i2 = p0;
      i3 = l5;
      i4 = l6;
      i3 = i3 != i4;
      i2 |= i3;
      i1 &= i2;
      d2 = l23;
      d3 = 0;
      i2 = d2 != d3;
      i1 &= i2;
      i1 = !(i1);
      if (i1) {goto B80;}
    }
    i0 = 5200u;
    i1 = 34u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    d0 = l24;
    B80:;
  } else {
    d0 = l24;
  }
  i1 = l7;
  d0 = f64_0(d0, i1);
  B11:;
  l23 = d0;
  i0 = l17;
  g7 = i0;
  d0 = l23;
  FUNC_EPILOGUE;
  return d0;
}

static u64 f60(u32 p0) {
  u32 l1 = 0, l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0;
  u64 l7 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  u64 j0, j1, j2;
  i0 = p0;
  i1 = 4u;
  i0 += i1;
  l1 = i0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l2 = i0;
  i1 = p0;
  i2 = 100u;
  i1 += i2;
  l4 = i1;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i0 = i0 < i1;
  if (i0) {
    i0 = l1;
    i1 = l2;
    i2 = 1u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l2;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
  } else {
    i0 = p0;
    i0 = f32_0(i0);
  }
  l2 = i0;
  i1 = 43u;
  i0 -= i1;
  switch (i0) {
    case 0: goto B4;
    case 1: goto B3;
    case 2: goto B4;
    default: goto B3;
  }
  B4:;
  i0 = l2;
  i1 = 45u;
  i0 = i0 == i1;
  l6 = i0;
  i0 = l1;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l5 = i0;
  i1 = l4;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i0 = i0 < i1;
  if (i0) {
    i0 = l1;
    i1 = l5;
    i2 = 1u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l5;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
  } else {
    i0 = p0;
    i0 = f32_0(i0);
  }
  l5 = i0;
  i1 = 4294967248u;
  i0 += i1;
  l2 = i0;
  i1 = 9u;
  i0 = i0 > i1;
  i1 = 0u;
  i0 &= i1;
  if (i0) {
    i0 = l4;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    if (i0) {
      i0 = l1;
      i1 = l1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i2 = 4294967295u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      goto B1;
    } else {
      j0 = 9223372036854775808ull;
    }
  } else {
    i0 = l5;
    l3 = i0;
    goto B2;
  }
  goto B0;
  B3:;
  i0 = l2;
  l3 = i0;
  i0 = l2;
  i1 = 4294967248u;
  i0 += i1;
  l2 = i0;
  B2:;
  i0 = l2;
  i1 = 9u;
  i0 = i0 > i1;
  if (i0) {goto B1;}
  i0 = 0u;
  l2 = i0;
  L9: 
    i0 = l3;
    i1 = 4294967248u;
    i0 += i1;
    i1 = l2;
    i2 = 10u;
    i1 *= i2;
    i0 += i1;
    l2 = i0;
    i1 = 214748364u;
    i0 = (u32)((s32)i0 < (s32)i1);
    i1 = l1;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    l3 = i1;
    i2 = l4;
    i2 = i32_load(Z_envZ_memory, (u64)(i2));
    i1 = i1 < i2;
    if (i1) {
      i1 = l1;
      i2 = l3;
      i3 = 1u;
      i2 += i3;
      i32_store(Z_envZ_memory, (u64)(i1), i2);
      i1 = l3;
      i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
    } else {
      i1 = p0;
      i1 = f32_0(i1);
    }
    l3 = i1;
    i2 = 4294967248u;
    i1 += i2;
    l5 = i1;
    i2 = 10u;
    i1 = i1 < i2;
    i0 &= i1;
    if (i0) {goto L9;}
  i0 = l2;
  j0 = (u64)(s64)(s32)(i0);
  l7 = j0;
  i0 = l5;
  i1 = 10u;
  i0 = i0 < i1;
  if (i0) {
    L12: 
      i0 = l3;
      j0 = (u64)(s64)(s32)(i0);
      j1 = 18446744073709551568ull;
      j0 += j1;
      j1 = l7;
      j2 = 10ull;
      j1 *= j2;
      j0 += j1;
      l7 = j0;
      i0 = l1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      i1 = l4;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i0 = i0 < i1;
      if (i0) {
        i0 = l1;
        i1 = l3;
        i2 = 1u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l3;
        i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
      } else {
        i0 = p0;
        i0 = f32_0(i0);
      }
      l3 = i0;
      i1 = 4294967248u;
      i0 += i1;
      l2 = i0;
      i1 = 10u;
      i0 = i0 < i1;
      j1 = l7;
      j2 = 92233720368547758ull;
      i1 = (u64)((s64)j1 < (s64)j2);
      i0 &= i1;
      if (i0) {goto L12;}
    i0 = l2;
    i1 = 10u;
    i0 = i0 < i1;
    if (i0) {
      L15: 
        i0 = l1;
        i0 = i32_load(Z_envZ_memory, (u64)(i0));
        l3 = i0;
        i1 = l4;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i0 = i0 < i1;
        if (i0) {
          i0 = l1;
          i1 = l3;
          i2 = 1u;
          i1 += i2;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l3;
          i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
        } else {
          i0 = p0;
          i0 = f32_0(i0);
        }
        i1 = 4294967248u;
        i0 += i1;
        i1 = 10u;
        i0 = i0 < i1;
        if (i0) {goto L15;}
    }
  }
  i0 = l4;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  if (i0) {
    i0 = l1;
    i1 = l1;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 4294967295u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  }
  j0 = 0ull;
  j1 = l7;
  j0 -= j1;
  j1 = l7;
  i2 = l6;
  j0 = i2 ? j0 : j1;
  goto B0;
  B1:;
  i0 = l4;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  if (i0) {
    i0 = l1;
    i1 = l1;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 4294967295u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    j0 = 9223372036854775808ull;
  } else {
    j0 = 9223372036854775808ull;
  }
  B0:;
  FUNC_EPILOGUE;
  return j0;
}

static f64 f61(f64 p0, u32 p1) {
  u32 l2 = 0, l3 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  u64 j1, j2;
  f64 d0, d1;
  i0 = p1;
  i1 = 1023u;
  i0 = (u32)((s32)i0 > (s32)i1);
  if (i0) {
    d0 = p0;
    d1 = 8.9884656743115795e+307;
    d0 *= d1;
    p0 = d0;
    d1 = 8.9884656743115795e+307;
    d0 *= d1;
    d1 = p0;
    i2 = p1;
    i3 = 2046u;
    i2 = (u32)((s32)i2 > (s32)i3);
    l2 = i2;
    d0 = i2 ? d0 : d1;
    p0 = d0;
    i0 = p1;
    i1 = 4294965250u;
    i0 += i1;
    l3 = i0;
    i1 = 1023u;
    i2 = l3;
    i3 = 1023u;
    i2 = (u32)((s32)i2 < (s32)i3);
    i0 = i2 ? i0 : i1;
    i1 = p1;
    i2 = 4294966273u;
    i1 += i2;
    i2 = l2;
    i0 = i2 ? i0 : i1;
    p1 = i0;
  } else {
    i0 = p1;
    i1 = 4294966274u;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {
      d0 = p0;
      d1 = 2.2250738585072014e-308;
      d0 *= d1;
      p0 = d0;
      d1 = 2.2250738585072014e-308;
      d0 *= d1;
      d1 = p0;
      i2 = p1;
      i3 = 4294965252u;
      i2 = (u32)((s32)i2 < (s32)i3);
      l2 = i2;
      d0 = i2 ? d0 : d1;
      p0 = d0;
      i0 = p1;
      i1 = 2044u;
      i0 += i1;
      l3 = i0;
      i1 = 4294966274u;
      i2 = l3;
      i3 = 4294966274u;
      i2 = (u32)((s32)i2 > (s32)i3);
      i0 = i2 ? i0 : i1;
      i1 = p1;
      i2 = 1022u;
      i1 += i2;
      i2 = l2;
      i0 = i2 ? i0 : i1;
      p1 = i0;
    }
  }
  d0 = p0;
  i1 = p1;
  i2 = 1023u;
  i1 += i2;
  j1 = (u64)(i1);
  j2 = 52ull;
  j1 <<= (j2 & 63);
  d1 = f64_reinterpret_i64(j1);
  d0 *= d1;
  FUNC_EPILOGUE;
  return d0;
}

static f64 f62(f64 p0, f64 p1) {
  FUNC_PROLOGUE;
  u64 j0, j1, j2;
  f64 d0, d1;
  d0 = p0;
  j0 = i64_reinterpret_f64(d0);
  j1 = 9223372036854775807ull;
  j0 &= j1;
  d1 = p1;
  j1 = i64_reinterpret_f64(d1);
  j2 = 9223372036854775808ull;
  j1 &= j2;
  j0 |= j1;
  d0 = f64_reinterpret_i64(j0);
  FUNC_EPILOGUE;
  return d0;
}

static f64 f63(f64 p0, f64 p1) {
  FUNC_PROLOGUE;
  f64 d0, d1;
  d0 = p0;
  d1 = p1;
  d0 = f65(d0, d1);
  FUNC_EPILOGUE;
  return d0;
}

static f64 f64_0(f64 p0, u32 p1) {
  FUNC_PROLOGUE;
  u32 i1;
  f64 d0;
  d0 = p0;
  i1 = p1;
  d0 = f61(d0, i1);
  FUNC_EPILOGUE;
  return d0;
}

static f64 f65(f64 p0, f64 p1) {
  u32 l2 = 0, l3 = 0, l4 = 0;
  u64 l5 = 0, l6 = 0, l7 = 0, l8 = 0, l9 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  u64 j0, j1, j2, j3;
  f64 d0, d1;
  d0 = p0;
  j0 = i64_reinterpret_f64(d0);
  l6 = j0;
  j1 = 52ull;
  j0 >>= (j1 & 63);
  i0 = (u32)(j0);
  i1 = 2047u;
  i0 &= i1;
  l2 = i0;
  d0 = p1;
  j0 = i64_reinterpret_f64(d0);
  l7 = j0;
  j1 = 52ull;
  j0 >>= (j1 & 63);
  i0 = (u32)(j0);
  i1 = 2047u;
  i0 &= i1;
  l4 = i0;
  j0 = l6;
  j1 = 9223372036854775808ull;
  j0 &= j1;
  l8 = j0;
  j0 = l7;
  j1 = 1ull;
  j0 <<= (j1 & 63);
  l5 = j0;
  j1 = 0ull;
  i0 = j0 == j1;
  if (i0) {goto B1;}
  i0 = l2;
  i1 = 2047u;
  i0 = i0 == i1;
  d1 = p1;
  j1 = i64_reinterpret_f64(d1);
  j2 = 9223372036854775807ull;
  j1 &= j2;
  j2 = 9218868437227405312ull;
  i1 = j1 > j2;
  i0 |= i1;
  if (i0) {goto B1;}
  j0 = l6;
  j1 = 1ull;
  j0 <<= (j1 & 63);
  l9 = j0;
  j1 = l5;
  i0 = j0 <= j1;
  if (i0) {
    d0 = p0;
    d1 = 0;
    d0 *= d1;
    d1 = p0;
    j2 = l5;
    j3 = l9;
    i2 = j2 == j3;
    d0 = i2 ? d0 : d1;
    goto Bfunc;
  }
  i0 = l2;
  if (i0) {
    j0 = l6;
    j1 = 4503599627370495ull;
    j0 &= j1;
    j1 = 4503599627370496ull;
    j0 |= j1;
  } else {
    j0 = l6;
    j1 = 12ull;
    j0 <<= (j1 & 63);
    l5 = j0;
    j1 = 18446744073709551615ull;
    i0 = (u64)((s64)j0 > (s64)j1);
    if (i0) {
      i0 = 0u;
      l2 = i0;
      L6: 
        i0 = l2;
        i1 = 4294967295u;
        i0 += i1;
        l2 = i0;
        j0 = l5;
        j1 = 1ull;
        j0 <<= (j1 & 63);
        l5 = j0;
        j1 = 18446744073709551615ull;
        i0 = (u64)((s64)j0 > (s64)j1);
        if (i0) {goto L6;}
    } else {
      i0 = 0u;
      l2 = i0;
    }
    j0 = l6;
    i1 = 1u;
    i2 = l2;
    i1 -= i2;
    j1 = (u64)(i1);
    j0 <<= (j1 & 63);
  }
  l6 = j0;
  i1 = l4;
  if (i1) {
    j1 = l7;
    j2 = 4503599627370495ull;
    j1 &= j2;
    j2 = 4503599627370496ull;
    j1 |= j2;
  } else {
    j1 = l7;
    j2 = 12ull;
    j1 <<= (j2 & 63);
    l5 = j1;
    j2 = 18446744073709551615ull;
    i1 = (u64)((s64)j1 > (s64)j2);
    if (i1) {
      L9: 
        i1 = l3;
        i2 = 4294967295u;
        i1 += i2;
        l3 = i1;
        j1 = l5;
        j2 = 1ull;
        j1 <<= (j2 & 63);
        l5 = j1;
        j2 = 18446744073709551615ull;
        i1 = (u64)((s64)j1 > (s64)j2);
        if (i1) {goto L9;}
    }
    j1 = l7;
    i2 = 1u;
    i3 = l3;
    l4 = i3;
    i2 -= i3;
    j2 = (u64)(i2);
    j1 <<= (j2 & 63);
  }
  l7 = j1;
  j0 -= j1;
  l5 = j0;
  j1 = 18446744073709551615ull;
  i0 = (u64)((s64)j0 > (s64)j1);
  l3 = i0;
  i0 = l2;
  i1 = l4;
  i0 = (u32)((s32)i0 > (s32)i1);
  if (i0) {
    L12: 
      i0 = l3;
      if (i0) {
        j0 = l5;
        j1 = 0ull;
        i0 = j0 == j1;
        if (i0) {goto B13;}
      } else {
        j0 = l6;
        l5 = j0;
      }
      j0 = l5;
      j1 = 1ull;
      j0 <<= (j1 & 63);
      l6 = j0;
      j1 = l7;
      j0 -= j1;
      l5 = j0;
      j1 = 18446744073709551615ull;
      i0 = (u64)((s64)j0 > (s64)j1);
      l3 = i0;
      i0 = l2;
      i1 = 4294967295u;
      i0 += i1;
      l2 = i0;
      i1 = l4;
      i0 = (u32)((s32)i0 > (s32)i1);
      if (i0) {goto L12;}
      goto B11;
      B13:;
    d0 = p0;
    d1 = 0;
    d0 *= d1;
    goto B2;
    B11:;
  }
  i0 = l3;
  if (i0) {
    d0 = p0;
    d1 = 0;
    d0 *= d1;
    j1 = l5;
    j2 = 0ull;
    i1 = j1 == j2;
    if (i1) {goto B2;}
  } else {
    j0 = l6;
    l5 = j0;
  }
  j0 = l5;
  j1 = 4503599627370496ull;
  i0 = j0 < j1;
  if (i0) {
    L17: 
      i0 = l2;
      i1 = 4294967295u;
      i0 += i1;
      l2 = i0;
      j0 = l5;
      j1 = 1ull;
      j0 <<= (j1 & 63);
      l5 = j0;
      j1 = 4503599627370496ull;
      i0 = j0 < j1;
      if (i0) {goto L17;}
  }
  i0 = l2;
  i1 = 0u;
  i0 = (u32)((s32)i0 > (s32)i1);
  if (i0) {
    j0 = l5;
    j1 = 18442240474082181120ull;
    j0 += j1;
    i1 = l2;
    j1 = (u64)(i1);
    j2 = 52ull;
    j1 <<= (j2 & 63);
    j0 |= j1;
  } else {
    j0 = l5;
    i1 = 1u;
    i2 = l2;
    i1 -= i2;
    j1 = (u64)(i1);
    j0 >>= (j1 & 63);
  }
  j1 = l8;
  j0 |= j1;
  d0 = f64_reinterpret_i64(j0);
  B2:;
  goto B0;
  B1:;
  d0 = p0;
  d1 = p1;
  d0 *= d1;
  p0 = d0;
  d1 = p0;
  d0 /= d1;
  B0:;
  Bfunc:;
  FUNC_EPILOGUE;
  return d0;
}

static void f66(u32 p0, u32 p1) {
  u32 l2 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = g7;
  l2 = i0;
  i0 = g7;
  i1 = 16u;
  i0 += i1;
  g7 = i0;
  i0 = l2;
  i1 = p1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = 2016u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  i1 = p0;
  i2 = l2;
  f40(i0, i1, i2);
  i0 = l2;
  g7 = i0;
  FUNC_EPILOGUE;
}

static void f67(void) {
  FUNC_PROLOGUE;
  u32 i0;
  i0 = 2016u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  f68(i0);
  FUNC_EPILOGUE;
}

static void f68(u32 p0) {
  u32 l1 = 0, l2 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76));
  i1 = 0u;
  i0 = (u32)((s32)i0 >= (s32)i1);
  if (i0) {
    i0 = p0;
    i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 75));
    i1 = 10u;
    i0 = i0 == i1;
    if (i0) {goto B2;}
    i0 = p0;
    i1 = 20u;
    i0 += i1;
    l2 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l1 = i0;
    i1 = p0;
    i1 = i32_load(Z_envZ_memory, (u64)(i1 + 16));
    i0 = i0 >= i1;
    if (i0) {goto B2;}
    i0 = l2;
    i1 = l1;
    i2 = 1u;
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = l1;
    i1 = 10u;
    i32_store8(Z_envZ_memory, (u64)(i0), i1);
    goto B0;
    B2:;
    i0 = p0;
    f53(i0);
    goto B0;
  }
  i0 = p0;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 75));
  i1 = 10u;
  i0 = i0 != i1;
  if (i0) {
    i0 = p0;
    i1 = 20u;
    i0 += i1;
    l2 = i0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l1 = i0;
    i1 = p0;
    i1 = i32_load(Z_envZ_memory, (u64)(i1 + 16));
    i0 = i0 < i1;
    if (i0) {
      i0 = l2;
      i1 = l1;
      i2 = 1u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l1;
      i1 = 10u;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      goto B0;
    }
  }
  i0 = p0;
  f53(i0);
  B0:;
  FUNC_EPILOGUE;
}

static u32 _memcpy(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0, l5 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p2;
  i1 = 8192u;
  i0 = (u32)((s32)i0 >= (s32)i1);
  if (i0) {
    i0 = p0;
    i1 = p1;
    i2 = p2;
    i0 = (*Z_envZ__emscripten_memcpy_bigZ_iiii)(i0, i1, i2);
    i0 = p0;
    goto Bfunc;
  }
  i0 = p0;
  l4 = i0;
  i0 = p0;
  i1 = p2;
  i0 += i1;
  l3 = i0;
  i0 = p0;
  i1 = 3u;
  i0 &= i1;
  i1 = p1;
  i2 = 3u;
  i1 &= i2;
  i0 = i0 == i1;
  if (i0) {
    L2: 
      i0 = p0;
      i1 = 3u;
      i0 &= i1;
      if (i0) {
        i0 = p2;
        i0 = !(i0);
        if (i0) {
          i0 = l4;
          goto Bfunc;
        }
        i0 = p0;
        i1 = p1;
        i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
        i32_store8(Z_envZ_memory, (u64)(i0), i1);
        i0 = p0;
        i1 = 1u;
        i0 += i1;
        p0 = i0;
        i0 = p1;
        i1 = 1u;
        i0 += i1;
        p1 = i0;
        i0 = p2;
        i1 = 1u;
        i0 -= i1;
        p2 = i0;
        goto L2;
      }
    i0 = l3;
    i1 = 4294967292u;
    i0 &= i1;
    p2 = i0;
    i1 = 4294967232u;
    i0 += i1;
    l5 = i0;
    L5: 
      i0 = p0;
      i1 = l5;
      i0 = (u32)((s32)i0 <= (s32)i1);
      if (i0) {
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
        i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 8));
        i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 12));
        i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 16));
        i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 20));
        i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 24));
        i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 28));
        i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 32));
        i32_store(Z_envZ_memory, (u64)(i0 + 32), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 36));
        i32_store(Z_envZ_memory, (u64)(i0 + 36), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 40));
        i32_store(Z_envZ_memory, (u64)(i0 + 40), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 44));
        i32_store(Z_envZ_memory, (u64)(i0 + 44), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 48));
        i32_store(Z_envZ_memory, (u64)(i0 + 48), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 52));
        i32_store(Z_envZ_memory, (u64)(i0 + 52), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 56));
        i32_store(Z_envZ_memory, (u64)(i0 + 56), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1 + 60));
        i32_store(Z_envZ_memory, (u64)(i0 + 60), i1);
        i0 = p0;
        i1 = 4294967232u;
        i0 -= i1;
        p0 = i0;
        i0 = p1;
        i1 = 4294967232u;
        i0 -= i1;
        p1 = i0;
        goto L5;
      }
    L7: 
      i0 = p0;
      i1 = p2;
      i0 = (u32)((s32)i0 < (s32)i1);
      if (i0) {
        i0 = p0;
        i1 = p1;
        i1 = i32_load(Z_envZ_memory, (u64)(i1));
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p0;
        i1 = 4u;
        i0 += i1;
        p0 = i0;
        i0 = p1;
        i1 = 4u;
        i0 += i1;
        p1 = i0;
        goto L7;
      }
  } else {
    i0 = l3;
    i1 = 4u;
    i0 -= i1;
    p2 = i0;
    L9: 
      i0 = p0;
      i1 = p2;
      i0 = (u32)((s32)i0 < (s32)i1);
      if (i0) {
        i0 = p0;
        i1 = p1;
        i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
        i32_store8(Z_envZ_memory, (u64)(i0), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 1));
        i32_store8(Z_envZ_memory, (u64)(i0 + 1), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 2));
        i32_store8(Z_envZ_memory, (u64)(i0 + 2), i1);
        i0 = p0;
        i1 = p1;
        i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 3));
        i32_store8(Z_envZ_memory, (u64)(i0 + 3), i1);
        i0 = p0;
        i1 = 4u;
        i0 += i1;
        p0 = i0;
        i0 = p1;
        i1 = 4u;
        i0 += i1;
        p1 = i0;
        goto L9;
      }
  }
  L11: 
    i0 = p0;
    i1 = l3;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {
      i0 = p0;
      i1 = p1;
      i1 = i32_load8_s(Z_envZ_memory, (u64)(i1));
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = 1u;
      i0 += i1;
      p0 = i0;
      i0 = p1;
      i1 = 1u;
      i0 += i1;
      p1 = i0;
      goto L11;
    }
  i0 = l4;
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static u32 _memset(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0, l5 = 0, l6 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  i1 = p2;
  i0 += i1;
  l4 = i0;
  i0 = p1;
  i1 = 255u;
  i0 &= i1;
  p1 = i0;
  i0 = p2;
  i1 = 67u;
  i0 = (u32)((s32)i0 >= (s32)i1);
  if (i0) {
    L1: 
      i0 = p0;
      i1 = 3u;
      i0 &= i1;
      if (i0) {
        i0 = p0;
        i1 = p1;
        i32_store8(Z_envZ_memory, (u64)(i0), i1);
        i0 = p0;
        i1 = 1u;
        i0 += i1;
        p0 = i0;
        goto L1;
      }
    i0 = p1;
    i1 = 8u;
    i0 <<= (i1 & 31);
    i1 = p1;
    i0 |= i1;
    i1 = p1;
    i2 = 16u;
    i1 <<= (i2 & 31);
    i0 |= i1;
    i1 = p1;
    i2 = 24u;
    i1 <<= (i2 & 31);
    i0 |= i1;
    l3 = i0;
    i0 = l4;
    i1 = 4294967292u;
    i0 &= i1;
    l5 = i0;
    i1 = 4294967232u;
    i0 += i1;
    l6 = i0;
    L3: 
      i0 = p0;
      i1 = l6;
      i0 = (u32)((s32)i0 <= (s32)i1);
      if (i0) {
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 32), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 36), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 40), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 44), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 48), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 52), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 56), i1);
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0 + 60), i1);
        i0 = p0;
        i1 = 4294967232u;
        i0 -= i1;
        p0 = i0;
        goto L3;
      }
    L5: 
      i0 = p0;
      i1 = l5;
      i0 = (u32)((s32)i0 < (s32)i1);
      if (i0) {
        i0 = p0;
        i1 = l3;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = p0;
        i1 = 4u;
        i0 += i1;
        p0 = i0;
        goto L5;
      }
  }
  L7: 
    i0 = p0;
    i1 = l4;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {
      i0 = p0;
      i1 = p1;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = 1u;
      i0 += i1;
      p0 = i0;
      goto L7;
    }
  i0 = l4;
  i1 = p2;
  i0 -= i1;
  FUNC_EPILOGUE;
  return i0;
}

static u32 _sbrk(u32 p0) {
  u32 l1 = 0, l2 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  i1 = g4;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  l2 = i1;
  i0 += i1;
  l1 = i0;
  i1 = l2;
  i0 = (u32)((s32)i0 < (s32)i1);
  i1 = p0;
  i2 = 0u;
  i1 = (u32)((s32)i1 > (s32)i2);
  i0 &= i1;
  i1 = l1;
  i2 = 0u;
  i1 = (u32)((s32)i1 < (s32)i2);
  i0 |= i1;
  if (i0) {
    i0 = l1;
    i0 = (*Z_envZ_abortOnCannotGrowMemoryZ_ii)(i0);
    i0 = 12u;
    (*Z_envZ____setErrNoZ_vi)(i0);
    i0 = 4294967295u;
    goto Bfunc;
  }
  i0 = l1;
  i1 = (*Z_envZ__emscripten_get_heap_sizeZ_iv)();
  i0 = (u32)((s32)i0 > (s32)i1);
  if (i0) {
    i0 = l1;
    i0 = (*Z_envZ__emscripten_resize_heapZ_ii)(i0);
    i0 = !(i0);
    if (i0) {
      i0 = 12u;
      (*Z_envZ____setErrNoZ_vi)(i0);
      i0 = 4294967295u;
      goto Bfunc;
    }
  }
  i0 = g4;
  i1 = l1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l2;
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static u32 dynCall_ii(u32 p0, u32 p1) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p1;
  i1 = p0;
  i2 = 1u;
  i1 &= i2;
  i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32), 1, i1, i0);
  FUNC_EPILOGUE;
  return i0;
}

static u32 dynCall_iiii(u32 p0, u32 p1, u32 p2, u32 p3) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  i0 = p1;
  i1 = p2;
  i2 = p3;
  i3 = p0;
  i4 = 7u;
  i3 &= i4;
  i4 = 2u;
  i3 += i4;
  i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, u32, u32), 0, i3, i0, i1, i2);
  FUNC_EPILOGUE;
  return i0;
}

static u32 f74(u32 p0) {
  FUNC_PROLOGUE;
  u32 i0;
  i0 = 0u;
  (*Z_envZ_abortZ_vi)(i0);
  i0 = 0u;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f75(u32 p0, u32 p1, u32 p2) {
  FUNC_PROLOGUE;
  u32 i0;
  i0 = 1u;
  (*Z_envZ_abortZ_vi)(i0);
  i0 = 0u;
  FUNC_EPILOGUE;
  return i0;
}

static const u8 data_segment_data_0[] = {
  0x02, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x04, 0x00, 0x00, 0xc0, 
  0x05, 0x00, 0x00, 0xc0, 0x06, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x00, 0xc0, 
  0x08, 0x00, 0x00, 0xc0, 0x09, 0x00, 0x00, 0xc0, 0x0a, 0x00, 0x00, 0xc0, 
  0x0b, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x00, 0xc0, 0x0d, 0x00, 0x00, 0xc0, 
  0x0e, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0xc0, 0x10, 0x00, 0x00, 0xc0, 
  0x11, 0x00, 0x00, 0xc0, 0x12, 0x00, 0x00, 0xc0, 0x13, 0x00, 0x00, 0xc0, 
  0x14, 0x00, 0x00, 0xc0, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0xc0, 
  0x17, 0x00, 0x00, 0xc0, 0x18, 0x00, 0x00, 0xc0, 0x19, 0x00, 0x00, 0xc0, 
  0x1a, 0x00, 0x00, 0xc0, 0x1b, 0x00, 0x00, 0xc0, 0x1c, 0x00, 0x00, 0xc0, 
  0x1d, 0x00, 0x00, 0xc0, 0x1e, 0x00, 0x00, 0xc0, 0x1f, 0x00, 0x00, 0xc0, 
  0x00, 0x00, 0x00, 0xb3, 0x01, 0x00, 0x00, 0xc3, 0x02, 0x00, 0x00, 0xc3, 
  0x03, 0x00, 0x00, 0xc3, 0x04, 0x00, 0x00, 0xc3, 0x05, 0x00, 0x00, 0xc3, 
  0x06, 0x00, 0x00, 0xc3, 0x07, 0x00, 0x00, 0xc3, 0x08, 0x00, 0x00, 0xc3, 
  0x09, 0x00, 0x00, 0xc3, 0x0a, 0x00, 0x00, 0xc3, 0x0b, 0x00, 0x00, 0xc3, 
  0x0c, 0x00, 0x00, 0xc3, 0x0d, 0x00, 0x00, 0xd3, 0x0e, 0x00, 0x00, 0xc3, 
  0x0f, 0x00, 0x00, 0xc3, 0x00, 0x00, 0x0c, 0xbb, 0x01, 0x00, 0x0c, 0xc3, 
  0x02, 0x00, 0x0c, 0xc3, 0x03, 0x00, 0x0c, 0xc3, 0x04, 0x00, 0x0c, 0xd3, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
  0x07, 0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x0b, 
  0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 
  0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
};

static const u8 data_segment_data_1[] = {
  0x11, 0x00, 0x0a, 0x00, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x05, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x0b, 
  
};

static const u8 data_segment_data_2[] = {
  0x11, 0x00, 0x0f, 0x0a, 0x11, 0x11, 0x11, 0x03, 0x0a, 0x07, 0x00, 0x01, 
  0x13, 0x09, 0x0b, 0x0b, 0x00, 0x00, 0x09, 0x06, 0x0b, 0x00, 0x00, 0x0b, 
  0x00, 0x06, 0x11, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 
};

static const u8 data_segment_data_3[] = {
  0x0b, 
};

static const u8 data_segment_data_4[] = {
  0x11, 0x00, 0x0a, 0x0a, 0x11, 0x11, 0x11, 0x00, 0x0a, 0x00, 0x00, 0x02, 
  0x00, 0x09, 0x0b, 0x00, 0x00, 0x00, 0x09, 0x00, 0x0b, 0x00, 0x00, 0x0b, 
  
};

static const u8 data_segment_data_5[] = {
  0x0c, 
};

static const u8 data_segment_data_6[] = {
  0x0c, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x09, 0x0c, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x0c, 
};

static const u8 data_segment_data_7[] = {
  0x0e, 
};

static const u8 data_segment_data_8[] = {
  0x0d, 0x00, 0x00, 0x00, 0x04, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x09, 0x0e, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x0e, 
};

static const u8 data_segment_data_9[] = {
  0x10, 
};

static const u8 data_segment_data_10[] = {
  0x0f, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x09, 0x10, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x12, 
  0x00, 0x00, 0x00, 0x12, 0x12, 0x12, 
};

static const u8 data_segment_data_11[] = {
  0x12, 0x00, 0x00, 0x00, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x09, 
};

static const u8 data_segment_data_12[] = {
  0x0b, 
};

static const u8 data_segment_data_13[] = {
  0x0a, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x09, 0x0b, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x0b, 
};

static const u8 data_segment_data_14[] = {
  0x0c, 
};

static const u8 data_segment_data_15[] = {
  0x0c, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x09, 0x0c, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x30, 
  0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 
  0x44, 0x45, 0x46, 0x0a, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0xe8, 
  0x03, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0xa0, 0x86, 0x01, 0x00, 0x40, 
  0x42, 0x0f, 0x00, 0x80, 0x96, 0x98, 0x00, 0x00, 0xe1, 0xf5, 0x05, 0xe4, 
  0x07, 0x00, 0x00, 0x05, 
};

static const u8 data_segment_data_16[] = {
  0x01, 
};

static const u8 data_segment_data_17[] = {
  0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x48, 0x0a, 0x00, 0x00, 
  0x00, 0x04, 
};

static const u8 data_segment_data_18[] = {
  0x01, 
};

static const u8 data_segment_data_19[] = {
  0x0a, 0xff, 0xff, 0xff, 0xff, 
};

static const u8 data_segment_data_20[] = {
  0x64, 0x08, 0x00, 0x00, 0x09, 
};

static const u8 data_segment_data_21[] = {
  0x01, 
};

static const u8 data_segment_data_22[] = {
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 
  0x58, 0x0e, 0x00, 0x00, 0x00, 0x04, 
};

static const u8 data_segment_data_23[] = {
  0xff, 0xff, 0xff, 0xff, 
};

static const u8 data_segment_data_24[] = {
  0x7c, 0x14, 
};

static const u8 data_segment_data_25[] = {
  0x5f, 0x70, 0x89, 0x00, 0xff, 0x09, 0x2f, 0x0f, 0x63, 0x6f, 0x6e, 0x67, 
  0x6c, 0x7a, 0x00, 0x74, 0x72, 0x79, 0x20, 0x61, 0x67, 0x61, 0x69, 0x6e, 
  0x00, 0x25, 0x73, 0x00, 0x00, 0x01, 0x02, 0x04, 0x07, 0x03, 0x06, 0x05, 
  0x00, 0x2d, 0x2b, 0x20, 0x20, 0x20, 0x30, 0x58, 0x30, 0x78, 0x00, 0x28, 
  0x6e, 0x75, 0x6c, 0x6c, 0x29, 0x00, 0x2d, 0x30, 0x58, 0x2b, 0x30, 0x58, 
  0x20, 0x30, 0x58, 0x2d, 0x30, 0x78, 0x2b, 0x30, 0x78, 0x20, 0x30, 0x78, 
  0x00, 0x69, 0x6e, 0x66, 0x00, 0x49, 0x4e, 0x46, 0x00, 0x4e, 0x41, 0x4e, 
  0x00, 0x2e, 0x00, 0x69, 0x6e, 0x66, 0x69, 0x6e, 0x69, 0x74, 0x79, 0x00, 
  0x6e, 0x61, 0x6e, 
};

static void init_memory(void) {
  memcpy(&((*Z_envZ_memory).data[1024u]), data_segment_data_0, 465);
  memcpy(&((*Z_envZ_memory).data[1504u]), data_segment_data_1, 24);
  memcpy(&((*Z_envZ_memory).data[1536u]), data_segment_data_2, 33);
  memcpy(&((*Z_envZ_memory).data[1585u]), data_segment_data_3, 1);
  memcpy(&((*Z_envZ_memory).data[1594u]), data_segment_data_4, 24);
  memcpy(&((*Z_envZ_memory).data[1643u]), data_segment_data_5, 1);
  memcpy(&((*Z_envZ_memory).data[1655u]), data_segment_data_6, 21);
  memcpy(&((*Z_envZ_memory).data[1701u]), data_segment_data_7, 1);
  memcpy(&((*Z_envZ_memory).data[1713u]), data_segment_data_8, 21);
  memcpy(&((*Z_envZ_memory).data[1759u]), data_segment_data_9, 1);
  memcpy(&((*Z_envZ_memory).data[1771u]), data_segment_data_10, 30);
  memcpy(&((*Z_envZ_memory).data[1826u]), data_segment_data_11, 14);
  memcpy(&((*Z_envZ_memory).data[1875u]), data_segment_data_12, 1);
  memcpy(&((*Z_envZ_memory).data[1887u]), data_segment_data_13, 21);
  memcpy(&((*Z_envZ_memory).data[1933u]), data_segment_data_14, 1);
  memcpy(&((*Z_envZ_memory).data[1945u]), data_segment_data_15, 76);
  memcpy(&((*Z_envZ_memory).data[2032u]), data_segment_data_16, 1);
  memcpy(&((*Z_envZ_memory).data[2056u]), data_segment_data_17, 14);
  memcpy(&((*Z_envZ_memory).data[2080u]), data_segment_data_18, 1);
  memcpy(&((*Z_envZ_memory).data[2095u]), data_segment_data_19, 5);
  memcpy(&((*Z_envZ_memory).data[2144u]), data_segment_data_20, 5);
  memcpy(&((*Z_envZ_memory).data[2160u]), data_segment_data_21, 1);
  memcpy(&((*Z_envZ_memory).data[2180u]), data_segment_data_22, 18);
  memcpy(&((*Z_envZ_memory).data[2224u]), data_segment_data_23, 4);
  memcpy(&((*Z_envZ_memory).data[2460u]), data_segment_data_24, 2);
  memcpy(&((*Z_envZ_memory).data[2516u]), data_segment_data_25, 99);
}

static void init_table(void) {
  uint32_t offset;
  offset = (*Z_envZ___table_baseZ_i);
  (*Z_envZ_table).data[offset + 0] = (wasm_rt_elem_t){func_types[1], (wasm_rt_anyfunc_t)(&f74)};
  (*Z_envZ_table).data[offset + 1] = (wasm_rt_elem_t){func_types[1], (wasm_rt_anyfunc_t)(&f22)};
  (*Z_envZ_table).data[offset + 2] = (wasm_rt_elem_t){func_types[0], (wasm_rt_anyfunc_t)(&f75)};
  (*Z_envZ_table).data[offset + 3] = (wasm_rt_elem_t){func_types[0], (wasm_rt_anyfunc_t)(&f27)};
  (*Z_envZ_table).data[offset + 4] = (wasm_rt_elem_t){func_types[0], (wasm_rt_anyfunc_t)(&f24)};
  (*Z_envZ_table).data[offset + 5] = (wasm_rt_elem_t){func_types[0], (wasm_rt_anyfunc_t)(&f28)};
  (*Z_envZ_table).data[offset + 6] = (wasm_rt_elem_t){func_types[0], (wasm_rt_anyfunc_t)(&f23)};
  (*Z_envZ_table).data[offset + 7] = (wasm_rt_elem_t){func_types[0], (wasm_rt_anyfunc_t)(&f75)};
  (*Z_envZ_table).data[offset + 8] = (wasm_rt_elem_t){func_types[0], (wasm_rt_anyfunc_t)(&f75)};
  (*Z_envZ_table).data[offset + 9] = (wasm_rt_elem_t){func_types[0], (wasm_rt_anyfunc_t)(&f75)};
}

/* export: '___errno_location' */
u32 (*WASM_RT_ADD_PREFIX(Z____errno_locationZ_iv))(void);
/* export: '_free' */
void (*WASM_RT_ADD_PREFIX(Z__freeZ_vi))(u32);
/* export: '_main' */
u32 (*WASM_RT_ADD_PREFIX(Z__mainZ_iv))(void);
/* export: '_malloc' */
u32 (*WASM_RT_ADD_PREFIX(Z__mallocZ_ii))(u32);
/* export: '_memcpy' */
u32 (*WASM_RT_ADD_PREFIX(Z__memcpyZ_iiii))(u32, u32, u32);
/* export: '_memset' */
u32 (*WASM_RT_ADD_PREFIX(Z__memsetZ_iiii))(u32, u32, u32);
/* export: '_sbrk' */
u32 (*WASM_RT_ADD_PREFIX(Z__sbrkZ_ii))(u32);
/* export: 'dynCall_ii' */
u32 (*WASM_RT_ADD_PREFIX(Z_dynCall_iiZ_iii))(u32, u32);
/* export: 'dynCall_iiii' */
u32 (*WASM_RT_ADD_PREFIX(Z_dynCall_iiiiZ_iiiii))(u32, u32, u32, u32);
/* export: 'establishStackSpace' */
void (*WASM_RT_ADD_PREFIX(Z_establishStackSpaceZ_vii))(u32, u32);
/* export: 'stackAlloc' */
u32 (*WASM_RT_ADD_PREFIX(Z_stackAllocZ_ii))(u32);
/* export: 'stackRestore' */
void (*WASM_RT_ADD_PREFIX(Z_stackRestoreZ_vi))(u32);
/* export: 'stackSave' */
u32 (*WASM_RT_ADD_PREFIX(Z_stackSaveZ_iv))(void);

static void init_exports(void) {
  /* export: '___errno_location' */
  WASM_RT_ADD_PREFIX(Z____errno_locationZ_iv) = (&___errno_location);
  /* export: '_free' */
  WASM_RT_ADD_PREFIX(Z__freeZ_vi) = (&_free);
  /* export: '_main' */
  WASM_RT_ADD_PREFIX(Z__mainZ_iv) = (&_main);
  /* export: '_malloc' */
  WASM_RT_ADD_PREFIX(Z__mallocZ_ii) = (&_malloc);
  /* export: '_memcpy' */
  WASM_RT_ADD_PREFIX(Z__memcpyZ_iiii) = (&_memcpy);
  /* export: '_memset' */
  WASM_RT_ADD_PREFIX(Z__memsetZ_iiii) = (&_memset);
  /* export: '_sbrk' */
  WASM_RT_ADD_PREFIX(Z__sbrkZ_ii) = (&_sbrk);
  /* export: 'dynCall_ii' */
  WASM_RT_ADD_PREFIX(Z_dynCall_iiZ_iii) = (&dynCall_ii);
  /* export: 'dynCall_iiii' */
  WASM_RT_ADD_PREFIX(Z_dynCall_iiiiZ_iiiii) = (&dynCall_iiii);
  /* export: 'establishStackSpace' */
  WASM_RT_ADD_PREFIX(Z_establishStackSpaceZ_vii) = (&establishStackSpace);
  /* export: 'stackAlloc' */
  WASM_RT_ADD_PREFIX(Z_stackAllocZ_ii) = (&stackAlloc);
  /* export: 'stackRestore' */
  WASM_RT_ADD_PREFIX(Z_stackRestoreZ_vi) = (&stackRestore);
  /* export: 'stackSave' */
  WASM_RT_ADD_PREFIX(Z_stackSaveZ_iv) = (&stackSave);
}

void WASM_RT_ADD_PREFIX(init)(void) {
  init_func_types();
  init_globals();
  init_memory();
  init_table();
  init_exports();
}
