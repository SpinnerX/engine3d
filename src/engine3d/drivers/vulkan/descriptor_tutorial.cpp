#include "drivers/vulkan/vulkan_context.hpp"
#include "engine_logger.hpp"
#include <drivers/vulkan/descriptor_tutorial.hpp>
#include <vulkan/vulkan_core.h>
#include <drivers/vulkan/helper_functions.hpp>

namespace engine3d::vk{
    /*
    DescriptorLayout::Builder DescriptorLayout::Builder::AddBinding(
                uint32_t binding,
                VkDescriptorType DescriptorType,
                VkShaderStageFlags StageFlags,
                uint32_t Count){
        VkDescriptorSetLayoutBinding layout_binding = {
            .binding = binding,
            .descriptorType = DescriptorType,
            .descriptorCount = Count,
            .stageFlags = StageFlags
        };
        bindings[binding] = layout_binding;
        return *this;
    }
            
    Ref<DescriptorLayout> DescriptorLayout::Builder::Build() const{
        return CreateRef<DescriptorLayout>(bindings);
    }

    DescriptorLayout::DescriptorLayout(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings){
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for(auto kv : bindings){
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
            .pBindings = setLayoutBindings.data()
        };

        vk_check(vkCreateDescriptorSetLayout(VulkanContext::GetDriver(), &ci, nullptr, &DescriptorSetLayout), "vkCreateDescriptorSetLayout", __FILE__, __LINE__, __FUNCTION__);


    }

    DescriptorLayout::~DescriptorLayout(){}


    DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType Type, uint32_t Count){
        m_PoolSizes.push_back({Type, Count});
        return *this;
    }
    
    DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags Flags){
        m_PoolFlags = Flags;
        return *this;
    }

    DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t Count){
        MaxSets = Count;

        return *this;
    }

    Ref<DescriptorPool> DescriptorPool::Builder::Build() const {
        return CreateRef<DescriptorPool>(MaxSets, m_PoolFlags, m_PoolSizes);
    }



    DescriptorPool::DescriptorPool(uint32_t MaxSets, VkDescriptorPoolCreateFlags PoolFlags, const std::vector<VkDescriptorPoolSize>& PoolSizes){
        VkDescriptorPoolCreateInfo pool_ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = PoolFlags,
            .maxSets = MaxSets,
            .poolSizeCount = static_cast<uint32_t>(PoolSizes.size()),
        };

        vk_check(vkCreateDescriptorPool(VulkanContext::GetDriver(), &pool_ci, nullptr, &m_DescriptorPool), "vkCreateDescriptorPool", __FILE__, __LINE__, __FUNCTION__);


    }

    bool DescriptorPool::AllocvateDescriptor(const VkDescriptorSetLayout DescriptorSetLayout, VkDescriptorSet& Descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = m_DescriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &DescriptorSetLayout,
        };

        vk_check(vkAllocateDescriptorSets(VulkanContext::GetDriver(), &allocInfo, &Descriptor), "vkAllocateDescriptorSets", __FILE__, __LINE__, __FUNCTION__);

        return true;
    }

    void DescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& Descriptors) const{
        vkFreeDescriptorSets(VulkanContext::GetDriver(), m_DescriptorPool, static_cast<uint32_t>(Descriptors.size()), Descriptors.data());
    }

    void DescriptorPool::Reset(){
        vkResetDescriptorPool(VulkanContext::GetDriver(), m_DescriptorPool, 0);
    }


    DescriptorWriter::DescriptorWriter(DescriptorLayout& setLayout, DescriptorPool& Pool) : m_Pool{Pool}, m_DescSetLayout{setLayout}{}

    DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo){
        auto& bindDescription = m_DescSetLayout.Get(binding);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;
        
        m_Writes.push_back(write);
        return *this;
    }

    DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo){
        auto& bindDescription = m_DescSetLayout.Get(binding);
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;
        
        m_Writes.push_back(write);
        return *this;
    }

    bool DescriptorWriter::Build(VkDescriptorSet& Set){
        ConsoleLogTrace("Before Part 3.1");
        bool success = m_Pool.AllocvateDescriptor(m_DescSetLayout.GetDescriptorSetLayout(), Set);

        if(!success){
            return false;
        }

        ConsoleLogTrace("Before Part #4");

        Overwrite(Set);

        ConsoleLogTrace("Before Part #5");
        return true;
    }

    void DescriptorWriter::Overwrite(VkDescriptorSet& Set){
        for (auto &write : m_Writes) {
            write.dstSet = Set;
        }
        vkUpdateDescriptorSets(VulkanContext::GetDriver(), m_Writes.size(), m_Writes.data(), 0, nullptr);
    }

    */
   // *************** Descriptor Set Layout Builder *********************
 
    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count) {
    assert(bindings.count(binding) == 0 && "Binding already in use");
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    bindings[binding] = layoutBinding;
    return *this;
    }
    
    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
    return std::make_unique<DescriptorSetLayout>(bindings);
    }
    
    // *************** Descriptor Set Layout *********************
    
    DescriptorSetLayout::DescriptorSetLayout(
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : bindings{bindings} {
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto kv : bindings) {
        setLayoutBindings.push_back(kv.second);
    }
    
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
    
    if (vkCreateDescriptorSetLayout(
            VulkanContext::GetDriver(),
            &descriptorSetLayoutInfo,
            nullptr,
            &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    }
    
    DescriptorSetLayout::~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(VulkanContext::GetDriver(), descriptorSetLayout, nullptr);
    }
    
    // *************** Descriptor Pool Builder *********************
    
    DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count) {
    poolSizes.push_back({descriptorType, count});
    return *this;
    }
    
    DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags) {
    poolFlags = flags;
    return *this;
    }
    DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
    maxSets = count;
    return *this;
    }
    
    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
    return std::make_unique<DescriptorPool>(maxSets, poolFlags, poolSizes);
    }
    
    // *************** Descriptor Pool *********************
    
    DescriptorPool::DescriptorPool(
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize> &poolSizes){
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;
    
    if (vkCreateDescriptorPool(VulkanContext::GetDriver(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    }
    
    DescriptorPool::~DescriptorPool() {
    vkDestroyDescriptorPool(VulkanContext::GetDriver(), descriptorPool, nullptr);
    }
    
    bool DescriptorPool::allocateDescriptor(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;
    
    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(VulkanContext::GetDriver(), &allocInfo, &descriptor) != VK_SUCCESS) {
        return false;
    }
    return true;
    }
    
    void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
    vkFreeDescriptorSets(
        VulkanContext::GetDriver(),
        descriptorPool,
        static_cast<uint32_t>(descriptors.size()),
        descriptors.data());
    }
    
    void DescriptorPool::resetPool() {
    vkResetDescriptorPool(VulkanContext::GetDriver(), descriptorPool, 0);
    }
    
    // *************** Descriptor Writer *********************
    
    DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool)
        : setLayout{setLayout}, pool{pool} {}
    
    DescriptorWriter &DescriptorWriter::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
    
    auto &bindingDescription = setLayout.Get(binding);
    
    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple");
    
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;
    
    writes.push_back(write);
    return *this;
    }
    
    DescriptorWriter &DescriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo *imageInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
    
    auto &bindingDescription = setLayout.Get(binding);
    
    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple");
    
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;
    
    writes.push_back(write);
    return *this;
    }
    
    bool DescriptorWriter::build(VkDescriptorSet &set) {
    bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
    if (!success) {
        return false;
    }
    overwrite(set);
    return true;
    }
    
    void DescriptorWriter::overwrite(VkDescriptorSet &set) {
    for (auto &write : writes) {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(VulkanContext::GetDriver(), writes.size(), writes.data(), 0, nullptr);
    }
};