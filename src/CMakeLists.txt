
add_library(Primitive STATIC)
add_library(LGL::Primitive ALIAS Primitive)

target_include_directories(Primitive
    PRIVATE
    basic/include
    common/include
    PUBLIC
    basic/interface
    common/interface
)

target_sources(Primitive
    PRIVATE
    basic/src/SpinLock.cpp
    common/src/DataBlob.cpp
    common/src/DefaultMemoryAllocator.cpp
    common/src/Time.cpp
    common/src/VariableSizeAllocationsManager.cpp
    PUBLIC
    basic/interface/Align.hpp
    basic/interface/Atomics.hpp
    basic/interface/CommonDefines.hpp
    basic/interface/FormatString.hpp
    basic/interface/HashUtils.hpp
    basic/interface/IAllocator.hpp
    basic/interface/IDataBlob.hpp
    basic/interface/InterfaceUUID.hpp
    basic/interface/IObject.hpp
    basic/interface/IReferenceCounter.hpp
    basic/interface/Logger.hpp
    basic/interface/Misc.hpp
    basic/interface/SpinLock.hpp

    common/interface/CStringTool.hpp
    common/interface/DataBlob.hpp
    common/interface/DebugUtility.hpp
    common/interface/DefaultMemoryAllocator.hpp
    common/interface/LoadDll.hpp
    common/interface/RefCountPtr.hpp
    common/interface/RefCountWeakPtr.hpp
    common/interface/ReferenceCounter.hpp
    common/interface/ReferenceCounterObject.hpp
    common/interface/STDAllocator.hpp
    common/interface/Time.hpp
    common/interface/TObject.hpp
    common/interface/VariableSizeAllocationsManager.hpp
)

target_link_libraries(Primitive
    PRIVATE
    BuildSetting
)

L_target_set_cpp(Primitive 20)

L_install_libraries(Primitive)