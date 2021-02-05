/*
 *  Copyright 2019-2021 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#pragma once

/// \file
/// Declaration of Diligent::RenderDeviceD3D12Impl class
#include "RenderDeviceD3D12.h"
#include "RenderDeviceD3DBase.hpp"
#include "RenderDeviceNextGenBase.hpp"
#include "DescriptorHeap.hpp"
#include "CommandListManager.hpp"
#include "CommandContext.hpp"
#include "D3D12DynamicHeap.hpp"
#include "Atomics.hpp"
#include "CommandQueueD3D12.h"
#include "GenerateMips.hpp"
#include "QueryManagerD3D12.hpp"
#include "DXCompiler.hpp"
#include "RootSignature.hpp"

// The macros below are only defined in Win SDK 19041+ and are missing in 17763
#ifndef D3D12_RAYTRACING_MAX_RAY_GENERATION_SHADER_THREADS
#    define D3D12_RAYTRACING_MAX_RAY_GENERATION_SHADER_THREADS (1073741824)
#endif
#ifndef D3D12_RAYTRACING_MAX_SHADER_RECORD_STRIDE
#    define D3D12_RAYTRACING_MAX_SHADER_RECORD_STRIDE (4096)
#endif
#ifndef D3D12_RAYTRACING_MAX_INSTANCES_PER_TOP_LEVEL_ACCELERATION_STRUCTURE
#    define D3D12_RAYTRACING_MAX_INSTANCES_PER_TOP_LEVEL_ACCELERATION_STRUCTURE (16777216)
#endif
#ifndef D3D12_RAYTRACING_MAX_PRIMITIVES_PER_BOTTOM_LEVEL_ACCELERATION_STRUCTURE
#    define D3D12_RAYTRACING_MAX_PRIMITIVES_PER_BOTTOM_LEVEL_ACCELERATION_STRUCTURE (536870912)
#endif
#ifndef D3D12_RAYTRACING_MAX_GEOMETRIES_PER_BOTTOM_LEVEL_ACCELERATION_STRUCTURE
#    define D3D12_RAYTRACING_MAX_GEOMETRIES_PER_BOTTOM_LEVEL_ACCELERATION_STRUCTURE (16777216)
#endif


namespace Diligent
{

/// Render device implementation in Direct3D12 backend.
class RenderDeviceD3D12Impl final : public RenderDeviceNextGenBase<RenderDeviceD3DBase<IRenderDeviceD3D12>, ICommandQueueD3D12>
{
public:
    using TRenderDeviceBase = RenderDeviceNextGenBase<RenderDeviceD3DBase<IRenderDeviceD3D12>, ICommandQueueD3D12>;

    RenderDeviceD3D12Impl(IReferenceCounters*          pRefCounters,
                          IMemoryAllocator&            RawMemAllocator,
                          IEngineFactory*              pEngineFactory,
                          const EngineD3D12CreateInfo& EngineCI,
                          ID3D12Device*                pD3D12Device,
                          size_t                       CommandQueueCount,
                          ICommandQueueD3D12**         ppCmdQueues) noexcept(false);
    ~RenderDeviceD3D12Impl();

    IMPLEMENT_QUERY_INTERFACE_IN_PLACE(IID_RenderDeviceD3D12, TRenderDeviceBase)

    /// Implementation of IRenderDevice::CreateGraphicsPipelineState() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateGraphicsPipelineState(const GraphicsPipelineStateCreateInfo& PSOCreateInfo, IPipelineState** ppPipelineState) override final;

    /// Implementation of IRenderDevice::CreateComputePipelineState() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateComputePipelineState(const ComputePipelineStateCreateInfo& PSOCreateInfo, IPipelineState** ppPipelineState) override final;

    /// Implementation of IRenderDevice::CreateRayTracingPipelineState() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateRayTracingPipelineState(const RayTracingPipelineStateCreateInfo& PSOCreateInfo, IPipelineState** ppPipelineState) override final;

    /// Implementation of IRenderDevice::CreateBuffer() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateBuffer(const BufferDesc& BuffDesc,
                                                 const BufferData* pBuffData,
                                                 IBuffer**         ppBuffer) override final;

    /// Implementation of IRenderDevice::CreateShader() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateShader(const ShaderCreateInfo& ShaderCreateInfo, IShader** ppShader) override final;

    /// Implementation of IRenderDevice::CreateTexture() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateTexture(const TextureDesc& TexDesc,
                                                  const TextureData* pData,
                                                  ITexture**         ppTexture) override final;

    void CreateTexture(const TextureDesc&       TexDesc,
                       ID3D12Resource*          pd3d12Texture,
                       RESOURCE_STATE           InitialState,
                       class TextureD3D12Impl** ppTexture);

    /// Implementation of IRenderDevice::CreateSampler() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateSampler(const SamplerDesc& SamplerDesc,
                                                  ISampler**         ppSampler) override final;

    /// Implementation of IRenderDevice::CreateFence() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateFence(const FenceDesc& Desc, IFence** ppFence) override final;

    /// Implementation of IRenderDevice::CreateQuery() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateQuery(const QueryDesc& Desc, IQuery** ppQuery) override final;

    /// Implementation of IRenderDevice::CreateRenderPass() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateRenderPass(const RenderPassDesc& Desc,
                                                     IRenderPass**         ppRenderPass) override final;

    /// Implementation of IRenderDevice::CreateFramebuffer() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateFramebuffer(const FramebufferDesc& Desc,
                                                      IFramebuffer**         ppFramebuffer) override final;

    /// Implementation of IRenderDevice::CreateBLAS() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateBLAS(const BottomLevelASDesc& Desc,
                                               IBottomLevelAS**         ppBLAS) override final;

    /// Implementation of IRenderDevice::CreateTLAS() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateTLAS(const TopLevelASDesc& Desc,
                                               ITopLevelAS**         ppTLAS) override final;

    /// Implementation of IRenderDevice::CreateSBT() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreateSBT(const ShaderBindingTableDesc& Desc,
                                              IShaderBindingTable**         ppSBT) override final;

    /// Implementation of IRenderDevice::CreatePipelineResourceSignature() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE CreatePipelineResourceSignature(const PipelineResourceSignatureDesc& Desc,
                                                                    IPipelineResourceSignature**         ppSignature) override final;

    void CreatePipelineResourceSignature(const PipelineResourceSignatureDesc& Desc,
                                         IPipelineResourceSignature**         ppSignature,
                                         bool                                 IsDeviceInternal);

    /// Implementation of IRenderDeviceD3D12::GetD3D12Device().
    virtual ID3D12Device* DILIGENT_CALL_TYPE GetD3D12Device() override final { return m_pd3d12Device; }

    /// Implementation of IRenderDeviceD3D12::CreateTextureFromD3DResource().
    virtual void DILIGENT_CALL_TYPE CreateTextureFromD3DResource(ID3D12Resource* pd3d12Texture,
                                                                 RESOURCE_STATE  InitialState,
                                                                 ITexture**      ppTexture) override final;

    /// Implementation of IRenderDeviceD3D12::CreateBufferFromD3DResource().
    virtual void DILIGENT_CALL_TYPE CreateBufferFromD3DResource(ID3D12Resource*   pd3d12Buffer,
                                                                const BufferDesc& BuffDesc,
                                                                RESOURCE_STATE    InitialState,
                                                                IBuffer**         ppBuffer) override final;

    /// Implementation of IRenderDeviceD3D12::CreateBLASFromD3DResource().
    virtual void DILIGENT_CALL_TYPE CreateBLASFromD3DResource(ID3D12Resource*          pd3d12BLAS,
                                                              const BottomLevelASDesc& Desc,
                                                              RESOURCE_STATE           InitialState,
                                                              IBottomLevelAS**         ppBLAS) override final;

    /// Implementation of IRenderDeviceD3D12::CreateTLASFromD3DResource().
    virtual void DILIGENT_CALL_TYPE CreateTLASFromD3DResource(ID3D12Resource*       pd3d12TLAS,
                                                              const TopLevelASDesc& Desc,
                                                              RESOURCE_STATE        InitialState,
                                                              ITopLevelAS**         ppTLAS) override final;

    void CreateRootSignature(const RefCntAutoPtr<class PipelineResourceSignatureD3D12Impl>* ppSignatures, Uint32 SignatureCount, RootSignatureD3D12** ppRootSig);

    RootSignatureCacheD3D12& GetRootSignatureCache() { return m_RootSignatureCache; }

    DescriptorHeapAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);
    DescriptorHeapAllocation AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);

    /// Implementation of IRenderDevice::IdleGPU() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE IdleGPU() override final;

    using PooledCommandContext = std::unique_ptr<CommandContext, STDDeleterRawMem<CommandContext>>;
    PooledCommandContext AllocateCommandContext(const Char* ID = "");

    void CloseAndExecuteTransientCommandContext(Uint32 CommandQueueIndex, PooledCommandContext&& Ctx);

    Uint64 CloseAndExecuteCommandContexts(Uint32                                                 QueueIndex,
                                          Uint32                                                 NumContexts,
                                          PooledCommandContext                                   pContexts[],
                                          bool                                                   DiscardStaleObjects,
                                          std::vector<std::pair<Uint64, RefCntAutoPtr<IFence>>>* pSignalFences);

    void SignalFences(Uint32 QueueIndex, std::vector<std::pair<Uint64, RefCntAutoPtr<IFence>>>& SignalFences);

    // Disposes an unused command context
    void DisposeCommandContext(PooledCommandContext&& Ctx);

    void FlushStaleResources(Uint32 CmdQueueIndex);

    /// Implementation of IRenderDevice::() in Direct3D12 backend.
    virtual void DILIGENT_CALL_TYPE ReleaseStaleResources(bool ForceRelease = false) override final;

    D3D12DynamicMemoryManager& GetDynamicMemoryManager() { return m_DynamicMemoryManager; }

    GPUDescriptorHeap& GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type)
    {
        VERIFY_EXPR(Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        return m_GPUDescriptorHeaps[Type];
    }

    const GenerateMipsHelper& GetMipsGenerator() const { return m_MipsGenerator; }
    QueryManagerD3D12&        GetQueryManager() { return m_QueryMgr; }

    IDXCompiler* GetDxCompiler() const { return m_pDxCompiler.get(); }

    ID3D12Device2* GetD3D12Device2();
    ID3D12Device5* GetD3D12Device5();

    struct Properties
    {
        const Uint32 ShaderGroupHandleSize       = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        const Uint32 MaxShaderRecordStride       = D3D12_RAYTRACING_MAX_SHADER_RECORD_STRIDE;
        const Uint32 ShaderGroupBaseAlignment    = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
        const Uint32 MaxDrawMeshTasksCount       = 64000; // from specs: https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html#dispatchmesh-api
        const Uint32 MaxRayTracingRecursionDepth = D3D12_RAYTRACING_MAX_DECLARABLE_TRACE_RECURSION_DEPTH;
        const Uint32 MaxRayGenThreads            = D3D12_RAYTRACING_MAX_RAY_GENERATION_SHADER_THREADS;
        const Uint32 MaxInstancesPerTLAS         = D3D12_RAYTRACING_MAX_INSTANCES_PER_TOP_LEVEL_ACCELERATION_STRUCTURE;
        const Uint32 MaxPrimitivesPerBLAS        = D3D12_RAYTRACING_MAX_PRIMITIVES_PER_BOTTOM_LEVEL_ACCELERATION_STRUCTURE;
        const Uint32 MaxGeometriesPerBLAS        = D3D12_RAYTRACING_MAX_GEOMETRIES_PER_BOTTOM_LEVEL_ACCELERATION_STRUCTURE;

        ShaderVersion MaxShaderVersion;
    };

    const Properties& GetProperties() const
    {
        return m_Properties;
    }

private:
    template <typename PSOCreateInfoType>
    void CreatePipelineState(const PSOCreateInfoType& PSOCreateInfo, IPipelineState** ppPipelineState);

    virtual void TestTextureFormat(TEXTURE_FORMAT TexFormat) override final;
    void         FreeCommandContext(PooledCommandContext&& Ctx);

    CComPtr<ID3D12Device> m_pd3d12Device;

    CComPtr<ID3D12Device2> m_pd3d12Device2;
    CComPtr<ID3D12Device5> m_pd3d12Device5;

    EngineD3D12CreateInfo m_EngineAttribs;

    CPUDescriptorHeap m_CPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    GPUDescriptorHeap m_GPUDescriptorHeaps[2]; // D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV == 0
                                               // D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER	 == 1

    CommandListManager m_CmdListManager;

    std::mutex                                                                  m_ContextPoolMutex;
    std::vector<PooledCommandContext, STDAllocatorRawMem<PooledCommandContext>> m_ContextPool;
#ifdef DILIGENT_DEVELOPMENT
    Atomics::AtomicLong m_AllocatedCtxCounter = 0;
#endif

    D3D12DynamicMemoryManager m_DynamicMemoryManager;

    // Note: mips generator must be released after the device has been idled
    GenerateMipsHelper m_MipsGenerator;

    QueryManagerD3D12 m_QueryMgr;

    Properties m_Properties;

    std::unique_ptr<IDXCompiler> m_pDxCompiler;

    FixedBlockMemoryAllocator m_RootSignatureAllocator;
    RootSignatureCacheD3D12   m_RootSignatureCache;
};

} // namespace Diligent
