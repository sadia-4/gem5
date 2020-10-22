/*
 * Copyright 2020 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ARCH_X86_LINUX_SE_WORKLOAD_HH__
#define __ARCH_X86_LINUX_SE_WORKLOAD_HH__

#include "arch/x86/linux/linux.hh"
#include "params/X86EmuLinux.hh"
#include "sim/se_workload.hh"
#include "sim/syscall_abi.hh"

namespace X86ISA
{

class EmuLinux : public SEWorkload
{
  public:
    using Params = X86EmuLinuxParams;

  protected:
    const Params &_params;

  public:
    const Params &params() const { return _params; }

    EmuLinux(const Params &p);

    Loader::Arch getArch() const override { return Loader::X86_64; }

    void syscall(ThreadContext *tc) override;

    struct SyscallABI64 :
        public GenericSyscallABI64, public X86Linux::SyscallABI
    {
        static const std::vector<IntRegIndex> ArgumentRegs;
    };

    struct SyscallABI32 :
        public GenericSyscallABI32, public X86Linux::SyscallABI
    {
        static const std::vector<IntRegIndex> ArgumentRegs;
    };
};

} // namespace X86ISA

namespace GuestABI
{

template <typename Arg>
struct Argument<X86ISA::EmuLinux::SyscallABI32, Arg,
    typename std::enable_if<
        X86ISA::EmuLinux::SyscallABI32::IsWide<Arg>::value>::type>
{
    using ABI = X86ISA::EmuLinux::SyscallABI32;

    static Arg
    get(ThreadContext *tc, typename ABI::State &state)
    {
        panic_if(state + 1 >= ABI::ArgumentRegs.size(),
                "Ran out of syscall argument registers.");
        auto low = ABI::ArgumentRegs[state++];
        auto high = ABI::ArgumentRegs[state++];
        return (Arg)ABI::mergeRegs(tc, low, high);
    }
};

} // namespace GuestABI

#endif // __ARCH_X86_LINUX_SE_WORKLOAD_HH__