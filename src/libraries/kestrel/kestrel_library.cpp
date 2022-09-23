// Copyright (c) 2022 Tom Hancocks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "libraries/kestrel/kestrel_library.hpp"

// MARK: - Root Import

auto kdl::builtin::libraries::kestrel::import(sema::parser &parser) -> void
{
    types::import_lua_script(parser);
    types::import_scene_interface(parser);
    types::import_sprite_set(parser);
    types::import_static_image(parser);
    types::import_package(parser);
    types::import_shader(parser);
    types::import_metal_shader(parser);
}

// MARK: - Type : LuaScript : LuaS

static const std::string s_lua_script_type {R"(
@type LuaScript : "LuaS" {
    template {
        CSTR Script;
    };

    field("Script") {
        Script as File;
    };
};
)"};

auto kdl::builtin::libraries::kestrel::types::import_lua_script(sema::parser &parser) -> void
{
    parser.import("Kestrel.LuaScript", s_lua_script_type);
}

// MARK: - Type : Shader : glsl

static const std::string s_shader_type {R"(
@type Shader : "glsl" {
    template {
        CSTR Shader;
    };

    field("Shader") {
        Shader as File;
    };
};
)"};

auto kdl::builtin::libraries::kestrel::types::import_shader(sema::parser &parser) -> void
{
    parser.import("Kestrel.Shader", s_shader_type);
}

// MARK: - Type : MetalShader : mlsl

static const std::string s_metal_shader_type {R"(
@type MetalShader : "mlsl" {
    template {
        CSTR Shader;
    };

    field("Shader") {
        Shader as File;
    };
};
)"};

auto kdl::builtin::libraries::kestrel::types::import_metal_shader(sema::parser &parser) -> void
{
    parser.import("Kestrel.MetalShader", s_metal_shader_type);
}

// MARK: - Type : SpriteSet : LuaS

static const std::string s_sprite_set_type {R"(
@type SpriteSet : "rlëX" {
    template {
        HEXD Data;
    };

    field("PNG") {
        Data as File<PNG> __conversion($InputFormat, rleX);
    };
    field("TGA") {
        Data as File<TGA> __conversion($InputFormat, rleX);
    };
};
)"};

auto kdl::builtin::libraries::kestrel::types::import_sprite_set(sema::parser &parser) -> void
{
    parser.import("Kestrel.SpriteSet", s_sprite_set_type);
}

// MARK: - Type : StaticImage : sïmg

static const std::string s_static_image_type {R"(
@type StaticImage : "sïmg" {
    template {
        HEXD Image;
    };

    field("PNG") {
        Data as File<PNG> __conversion($InputFormat, TGA);
    };
    field("TGA") {
        Data as File<TGA> __conversion($InputFormat, TGA);
    };
};
)"};

auto kdl::builtin::libraries::kestrel::types::import_static_image(sema::parser &parser) -> void
{
    parser.import("Kestrel.StaticImage", s_static_image_type);
}

// MARK: - Type : KestrelPackage : kmöd

static const std::string s_package_type {R"(
@type KestrelPackage : "kmöd" {
    template {
		CSTR Name;
		C040 Version;
		CSTR Author;
		CSTR PrimaryNamespace;
		DQAD LuaEntryScript;
		CSTR Description;
		CSTR Category;
		CSTR PackageID;
		CSTR ScenarioID;
	};
	field("Name") {
		Name;
	};
	field("Version") {
		Version;
	};
	field("Author") {
		Author;
	};
	field("PrimaryNamespace") {
		PrimaryNamespace;
	};
	field("LuaEntryScript") {
		LuaEntryScript as LuaScript&;
	};
	field("Description") {
		Description;
	};
	field("Category") {
		Category;
	};
	field("PackageID") {
		PackageID;
	};
	field("ScenarioID") {
		ScenarioID;
	};
};
)"};

auto kdl::builtin::libraries::kestrel::types::import_package(sema::parser &parser) -> void
{
    parser.import("Kestrel.KestrelPackage", s_package_type);
}

// MARK: - Type : KestrelPackage : kmöd

static const std::string s_scene_interface_type {R"(
@type SceneInterface : "scïn" {
	template {
		HWRD Flags;
		PSTR Title;
		DWRD SceneWidth;
		DWRD SceneHeight;
		OCNT Elements;
		LSTC ElementStart;
		HBYT	ElementType;
		PSTR 	ElementId;
		DWRD 	ElementX;
		DWRD 	ElementY;
		DWRD 	ElementWidth;
		DWRD 	ElementHeight;
		CSTR 	ElementValue;
		CSTR	ElementAction;
		HLNG	ElementChildCount;
		LSTE ElementEnd;
	};
	field("Flags") {
		Flags as Bitmask = 0 [
			UseImGui = 0x0001,
			ImGuiShowTitle = 0x0002,
			ImGuiCloseButton = 0x0004,
			ScenePassthrough = 0x0010,
		];
	};
	field("Title") {
		Title;
	};
	field("Size") {
		@name(width) SceneWidth;
		@name(height) SceneHeight;
	};
	field("Element") repeatable<0, 100, Elements> {
		@name(type) ElementType = None [
			None = 0,
			Button = 1,
			Label = 2,
			TextArea = 3,
			Image = 4,
			TextField = 5,
			Checkbox = 6,
			List = 7,
			ScrollArea = 8,
			Grid = 9,
			LabeledList = 10,
			Canvas = 11,
			Sprite = 12,
			PopupButton = 13,
			Slider = 14,
			Table = 15,
			Box = 16,
			Radio = 17,
			TabBar = 18,
			Separator = 19,
		];
		@name(id) ElementId = "";
		@name(x) ElementX = 0;
		@name(y) ElementY = 0;
		@name(width) ElementWidth = 0;
		@name(height) ElementHeight = 0;
		@name(value) ElementValue = "";
		@name(action) ElementAction = "";
		@name(childCount) ElementChildCount = 0;
	};
};
)"};

auto kdl::builtin::libraries::kestrel::types::import_scene_interface(sema::parser &parser) -> void
{
    parser.import("Kestrel.SceneInterface", s_scene_interface_type);
}