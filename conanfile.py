from conan import ConanFile
import os, sys, platform


class TinyPointersRecipe(ConanFile):
    name = "tiny_pointers"

    python_requires = "cor_recipe_utils/0.6.0"
    python_requires_extend = "cor_recipe_utils.ConanFileBase"

    tool_requires = [
        "cmake/[>=3.20.0]",
    ]

    settings = "os", "compiler", "build_type", "arch"

    #+++++++++++-+-+--+----- --- -- -  -  -   -
    # Optional metadata
    #
    license = "TODO"

    author = "TODO"

    url = "TODO"

    description = "TODO"

    topics = ("TODO",)
    #
    #+++++++++++-+-+--+----- --- -- -  -  -   -

    def requirements(self):
        VISIBLE = self.cor.VISIBLE
        OVERRIDE = self.cor.OVERRIDE

        self.requires("boost/[>=1.84.0]", **VISIBLE)
        self.test_requires("gtest/[>=1.14.0]")

    def configure(self):
        self.options["boost"].without_test = True

    #+++++++++++-+-+--+----- --- -- -  -  -   -

    def set_version(self):
        return self.cor.set_version_from_git_tags(self)

    def layout(self):
        return self.cor.layout_cmake_unified_src(self)

    def generate(self):
        return self.cor.generate_cmake_default(self)

    def build(self):
        return self.cor.build_cmake_default(self)

    def package(self):
        return self.cor.package_cmake_lib_default(self)

    def package_info(self):
        return self.cor.package_info_lib_default(self)

    def package_id(self):
        return self.cor.package_id_lib_default(self)

    #+++++++++++-+-+--+----- --- -- -  -  -   -
