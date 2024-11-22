        #include <cmrc/cmrc.hpp>
#include <map>
#include <utility>

namespace cmrc {
namespace depthai {

namespace res_chars {
// These are the files which are available in this resource library
// Pointers to C:/Users/vlad/code/moverse/depthai-core-v2.28.0/build/resources/depthai-device-fwp-9ed7c9ae4c232ff93a3500a585a6b1c00650e22c.tar.xz
extern const char* const f_9c23_depthai_device_fwp_9ed7c9ae4c232ff93a3500a585a6b1c00650e22c_tar_xz_begin;
extern const char* const f_9c23_depthai_device_fwp_9ed7c9ae4c232ff93a3500a585a6b1c00650e22c_tar_xz_end;
// Pointers to C:/Users/vlad/code/moverse/depthai-core-v2.28.0/build/resources/depthai-bootloader-fwp-0.0.28.tar.xz
extern const char* const f_322a_depthai_bootloader_fwp_0_0_28_tar_xz_begin;
extern const char* const f_322a_depthai_bootloader_fwp_0_0_28_tar_xz_end;
}

namespace {

const cmrc::detail::index_type&
get_root_index() {
    static cmrc::detail::directory root_directory_;
    static cmrc::detail::file_or_directory root_directory_fod{root_directory_};
    static cmrc::detail::index_type root_index;
    root_index.emplace("", &root_directory_fod);
    struct dir_inl {
        class cmrc::detail::directory& directory;
    };
    dir_inl root_directory_dir{root_directory_};
    (void)root_directory_dir;
    
    root_index.emplace(
        "depthai-device-fwp-9ed7c9ae4c232ff93a3500a585a6b1c00650e22c.tar.xz",
        root_directory_dir.directory.add_file(
            "depthai-device-fwp-9ed7c9ae4c232ff93a3500a585a6b1c00650e22c.tar.xz",
            res_chars::f_9c23_depthai_device_fwp_9ed7c9ae4c232ff93a3500a585a6b1c00650e22c_tar_xz_begin,
            res_chars::f_9c23_depthai_device_fwp_9ed7c9ae4c232ff93a3500a585a6b1c00650e22c_tar_xz_end
        )
    );
    root_index.emplace(
        "depthai-bootloader-fwp-0.0.28.tar.xz",
        root_directory_dir.directory.add_file(
            "depthai-bootloader-fwp-0.0.28.tar.xz",
            res_chars::f_322a_depthai_bootloader_fwp_0_0_28_tar_xz_begin,
            res_chars::f_322a_depthai_bootloader_fwp_0_0_28_tar_xz_end
        )
    );
    return root_index;
}

}

cmrc::embedded_filesystem get_filesystem() {
    static auto& index = get_root_index();
    return cmrc::embedded_filesystem{index};
}

} // depthai
} // cmrc
    