
add_library(Primitive STATIC)
add_library(LGL::Primitive ALIAS Primitive)

target_include_directories(Primitive
    PUBLIC
    utils/interface
    allocator/interface
    reference_counter/interface
)

target_sources(Primitive
    PRIVATE
    utils/src/SpinLock.cpp
    utils/src/Time.cpp
    utils/src/DataBlob.cpp
    utils/src/CString.cpp

    allocator/src/DefaultMemoryAllocator.cpp
    allocator/src/VariableSizeAllocationsManager.cpp

    PUBLIC
    allocator/interface/IAllocator.hpp
    allocator/interface/DefaultMemoryAllocator.hpp
    allocator/interface/VariableSizeAllocationsManager.hpp
    allocator/interface/STDAllocator.hpp

    reference_counter/interface/InterfaceUUID.hpp
    reference_counter/interface/IObject.hpp
    reference_counter/interface/IReferenceCounter.hpp
    reference_counter/interface/LoadDll.hpp
    reference_counter/interface/RefCountPtr.hpp
    reference_counter/interface/RefCountWeakPtr.hpp
    reference_counter/interface/ReferenceCounter.hpp
    reference_counter/interface/ReferenceCounterObject.hpp
    reference_counter/interface/TObject.hpp

    utils/interface/Align.hpp
    utils/interface/Atomics.hpp
    utils/interface/CommonDefines.hpp
    utils/interface/FormatString.hpp
    utils/interface/HashUtils.hpp
    utils/interface/IDataBlob.hpp
    utils/interface/Logger.hpp
    utils/interface/Misc.hpp
    utils/interface/SpinLock.hpp
    utils/interface/CStringTool.hpp
    utils/interface/DataBlob.hpp
    utils/interface/DebugUtility.hpp
    utils/interface/Time.hpp
    utils/interface/CString.hpp
)

target_link_libraries(Primitive
    PRIVATE
    BuildSetting
)

L_target_set_cpp(Primitive 20)

L_install_libraries(Primitive)