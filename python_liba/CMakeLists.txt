#[[
                для биндинга в питоновскую либу:
]]

# заставляем питон сказать где pybind11 (сначала сделайте pip install pybind11)
# ещё надо в clion в параметры cmake указать наш питон: -DPython_EXECUTABLE=C:/ProgramData/miniconda3/envs/main/python.exe
# (дальше туда же в site-packages будет скопирован linalg_py)
find_package(Python 3 REQUIRED COMPONENTS Interpreter Development.Module)
execute_process(
        COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
        OUTPUT_VARIABLE pybind11_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
)
file(TO_CMAKE_PATH "${pybind11_DIR}" pybind11_DIR)
find_package(pybind11 CONFIG REQUIRED)

# биндим линалг в питоновски-подключаемую либу (?)
pybind11_add_module(linalg_py bindings.cpp)
target_link_libraries(linalg_py PRIVATE linalg)

if(MINGW)  # компилирую через mingw
    # зашиваем libstdc++, libgcc и winpthread прямо в .pyd, чтобы не искать DLL
    target_link_options(linalg_py PRIVATE -static -static-libgcc -static-libstdc++)
    # русские сообщения в исключениях
    add_compile_options(/utf-8)
endif()

set(PY_OUT_DIR "${CMAKE_SOURCE_DIR}/build/py")
set_target_properties(linalg_py PROPERTIES
        OUTPUT_NAME linalg
        LIBRARY_OUTPUT_DIRECTORY         "${PY_OUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY         "${PY_OUT_DIR}"   # для .pyd на Windows
        LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${PY_OUT_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PY_OUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${PY_OUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PY_OUT_DIR}"
)

# после каждой сборки: .pyi для PyCharm (нужен pip install pybind11-stubgen в окружении main)
add_custom_command(TARGET linalg_py POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=$<TARGET_FILE_DIR:linalg_py>"
        "${Python_EXECUTABLE}" -m pybind11_stubgen linalg -o "$<TARGET_FILE_DIR:linalg_py>"
        COMMENT "Генерация linalg.pyi"
        VERBATIM
)

# опционально: копировать в site-packages окружения main
# (Python_SITELIB уже известен после find_package(Python), conda run не нужен)
add_custom_command(TARGET linalg_py POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_FILE_DIR:linalg_py>" "${Python_SITELIB}/"
        COMMENT "Копирование в site-packages"
        VERBATIM
)
