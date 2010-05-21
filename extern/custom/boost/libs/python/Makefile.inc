BOOST_PY_SRC = ${BOOST_INCLUDE_DIR}/libs/python/src
LIBNAME = boost_python
MAJOR = 1
MINOR = 43
PATCH = 0
SRC_CPP = ${BOOST_PY_SRC}/numeric.cpp \
          ${BOOST_PY_SRC}/list.cpp \
          ${BOOST_PY_SRC}/long.cpp \
          ${BOOST_PY_SRC}/dict.cpp \
          ${BOOST_PY_SRC}/tuple.cpp \
          ${BOOST_PY_SRC}/str.cpp \
          ${BOOST_PY_SRC}/slice.cpp \
          ${BOOST_PY_SRC}/converter/from_python.cpp \
          ${BOOST_PY_SRC}/converter/registry.cpp \
          ${BOOST_PY_SRC}/converter/type_id.cpp \
          ${BOOST_PY_SRC}/object/enum.cpp \
          ${BOOST_PY_SRC}/object/class.cpp \
          ${BOOST_PY_SRC}/object/function.cpp \
          ${BOOST_PY_SRC}/object/inheritance.cpp \
          ${BOOST_PY_SRC}/object/life_support.cpp \
          ${BOOST_PY_SRC}/object/pickle_support.cpp \
          ${BOOST_PY_SRC}/errors.cpp \
          ${BOOST_PY_SRC}/module.cpp \
          ${BOOST_PY_SRC}/converter/builtin_converters.cpp \
          ${BOOST_PY_SRC}/converter/arg_to_python_base.cpp \
          ${BOOST_PY_SRC}/object/iterator.cpp \
          ${BOOST_PY_SRC}/object/stl_iterator.cpp \
          ${BOOST_PY_SRC}/object_protocol.cpp \
          ${BOOST_PY_SRC}/object_operators.cpp \
          ${BOOST_PY_SRC}/wrapper.cpp \
          ${BOOST_PY_SRC}/import.cpp \
          ${BOOST_PY_SRC}/exec.cpp \
          ${BOOST_PY_SRC}/object/function_doc_signature.cpp
