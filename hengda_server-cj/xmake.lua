set_languages("c++11", "c11")
add_cxflags("-g3")
set_optimize("faster")  -- -O2
set_warnings("all")     -- -Wal
set_targetdir("./out")

add_includedirs("./Common")
add_includedirs("../include/co")

ddk = os.getenv("DDK_HOME")
print("ddk path: ", ddk)

add_includedirs(ddk .. "include/inc")
add_includedirs(ddk .. "include/libc_sec/include")
add_includedirs(ddk .. "include/third_party/opencv/include")
add_includedirs(ddk .. "include/third_party/cereal/include")
add_includedirs(ddk .. "include/third_party/protobuf/include")
add_includedirs(ddk .. "include/third_party/opencv/include")

add_includedirs("/opt/aarch64/ffmpeg/include")

after_build(function ()
    os.rm("build")
    os.cp("graph.config", "./out/")
    os.cp("network.json", "./out/")
    os.cp("saveVideo.sh", "./out/")
--    os.cp("models/ssd_512.om", "./out/")
end)

-- host
target("StreamPuller")
    set_kind("shared")
    add_files("StreamPuller/*.cpp")
    add_linkdirs(ddk .. "lib64")
    add_links("drvdevdrv", "drvhdc_host", "mmpa", "memory", "matrix")
    add_links("profilerclient", "slog", "c_sec", "protobuf", "ssl", "crypto")
    add_includedirs("/opt/aarch64/ffmpeg/include")
    add_linkdirs("/opt/aarch64/ffmpeg/lib")
    add_links("avformat")

target("DstEngine")
    set_kind("shared")
    add_files("DstEngine/*.cpp","utility/bordersloader.cpp","utility/json11.cpp")
    add_linkdirs("../lib/euler")
    add_links("base")
    add_linkdirs(ddk .. "lib64")
    add_links("drvdevdrv", "drvhdc_host", "mmpa", "memory", "matrix")
    add_links("profilerclient", "slog", "c_sec", "protobuf", "ssl", "crypto")

target("main")
    set_kind("binary")
    add_files("main.cpp","utility/*.cpp")
    add_linkdirs("../lib/euler")
    add_links("base")
    add_linkdirs(ddk .. "lib64")
    add_links("drvdevdrv","drvhdc_host" ,"mmpa", "memory", "matrix")
    add_links("profilerclient", "slog", "c_sec", "protobuf", "ssl", "crypto")
    add_syslinks("pthread","dl")

-- device	
target("SSDDetection")
    set_kind("shared")
    add_files("SSDDetection/*.cpp")
    add_linkdirs(ddk .. "device/lib")
    add_links("idedaemon", "Dvpp_api", "Dvpp_jpeg_decoder", "Dvpp_jpeg_encoder", "Dvpp_png_decoder", "Dvpp_vpc", "opencv_world")

target("JpegEncode")
    set_kind("shared")
    add_files("JpegEncode/*.cpp", "Common/DvppJpegEncode/*.cpp")
    add_linkdirs(ddk .. "device/lib")
    add_links("idedaemon", "Dvpp_api", "Dvpp_jpeg_decoder", "Dvpp_jpeg_encoder", "Dvpp_png_decoder", "Dvpp_vpc", "opencv_world")
    
target("VDecEngine")
    set_kind("shared")
    add_files("VDecEngine/*.cpp")
    add_linkdirs(ddk .. "device/lib")
    add_links("idedaemon", "Dvpp_api", "Dvpp_jpeg_decoder", "Dvpp_jpeg_encoder", "Dvpp_png_decoder", "Dvpp_vpc", "opencv_world")
    add_defines("VDecEngine_EXPORTS")
