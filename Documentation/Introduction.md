# Introduction to KDL

This guide will teach you the basics of using KDL, best practices and eventually more advanced concepts. There is a companion video series on YouTube to this guide, if you prefer to learn through a more visual format.

### Who is this guide for?
If you are interested in developing plugins or mods for Cosmic Frontier, or developing your own games for Kestrel then you should absolutely start by looking here.


### What is KDL?
KDL, or _Kestrel Definition Language_, is a bespoke definition language built for defining, constructing and laying out game data for games that run in the Kestrel engine. It can also be used for building content for the original Escape Velocity games or Classic Macintosh software.

KDL was originally born out of a need to interface with Resource Files from the Classic Macintosh environment, but gradually evolved into a tool that incorporates type safety, automatic resource id allocation, type associations, assertions, namespaces and much more.

Given that KDL is a purely text based language, it means that any KDL project is fully compatible with version control software such as _git_ or _mercurial_. It is recommended that you make use of version control whilst working on your KDL projects, as you would with any software project.

KDL is available on macOS, Windows and Linux.

---

## §1: Setting Up your Environment

## §2: KDL Basics
In this section we will start exploring the _Kestrel Definition Language_. By the end of the section, you should be able to define simple resource types and declare instances of them.

---
### §2.1: Hello World
It is essentially a tradition in programming to first write a "Hello, World!" program. Whilst KDL doesn't specifically allow for writing programs, we can make the assembler itself print out debug logging messages, so let's make use of that. This will give us a quick introduction into the syntax as well.

```kdl
` hello.kdl
` Prints the text "Hello, World!" to the console.
@out "Hello, World!";
```

There are a couple of things to explore in this. Let's step through line by line and see what everything is doing.

The first two lines are _comments_. Comments are fundamental aspects in any language and allow the user to insert text that will be ignored by the computer, but serve as notes to the user themselves and others. Here the first line is simply the file name, whilst the second line explains what the entire script is going to do.

The third and final line is what actually does anything. There are a few elements of note. The first thing to take note of is the semicolon `;` at the line. In KDL, all functional statements must be terminated by a semicolon.

The next thing to note is the leading `@` before the `out`. Technically these two things should be read as a single "word"; `@out`. Such "words" (with a leading `@`) are referred to as _Assembler Directives_, and they instruct the KDL assembler to immediately perform an action. In the case of the `@out` directive, it tells the assembler to write the specified content to the console/terminal.

In this example, that content is the text "Hello, World!". This text is technically referred to as a _String_. You can define a string by wrapping the desired text in double quotes `"`.

When the assembler encounters this line it will write "Hello, World!" out into the users console/terminal.

```sh
$ kdl hello.kdl
Hello, World!
```

---
### §2.2: Constructing Resources
Making the assembler write basic strings to the console/terminal is not particularly useful. It would be much more useful to be able to construct actual resources. For example we might want construct a list of strings.

```kdl
` strings.kdl
@import Macintosh;

` Construct a resource containing a list of Fruit names.
declare StringList {
	new (#128, "Fruit Names") {
		String = "Apple";
		String = "Banana";
		String = "Orange";
		String = "Pear";
		String = "Kiwi";
		String = "Melon";
		String = "Grapes";
	};
};
```

There is a bit more going on here than before. The first thing we encounter, other than a comment, is another _Assembler Directive_, `@import`. In this case the directive is responsible for importing _something_ into the current assembler session. The import directive can be used to import other KDL files or Built-in libraries. In this situation we are requesting to import the _Macintosh_ library, which defines a number of resource types found on the Classic Macintosh.

After importing the Macintosh library, we move on the real topic of this example. Constructing a new resource. In KDL, resource construction is referred to as _declaring_ a resource. The syntax for this is relatively straight forward.

```kdl
declare ResourceTypeName
```

The `declare` keyword indicates that we are preparing to construct resources of a specific type. It is then followed by the `ResourceTypeName`, which is the name of a resource type. In the example above, this name is `StringList`.

At this point we encounter an opening brace `{`. Everything in between this opening brace and its corresponding closing brace `}`, is considered part of the resource construction sequence. This sequence should contain a series of resource declarations.

A resource declaration typically begins with the `new` keyword. This informs the assembler that we are intending to construct a brand new resource.

```kdl
new (id, name)
```

The `new` keyword can accept a couple of associated values, which are used to specify the resource id and name.

Resource IDs were previously treated as simple integer types on Classic Macintosh. However KDL promotes resource IDs to a dedicated type, and they are denoted by the `#`. In the above example we have the ID `#128`. We will cover Resource IDs in much greater detail later on.

The name aspect of the resource declaration is simply a string that represents the name of resource.

Once again we encounter the opening and closing braces, with everything in between them being used to populate the new resource instance. In this section everything is provided as a sequence of fields with their corresponding values.

```
String = "Apple";
```

This particular resource type, `StringList`, contains a sequence of strings. This sequence can be populated by repeated providing a value to the "String" field, as is shown in the example.

---
### §2.3: Data Types
In the previous section we started to encounter a few different types of data; Strings, Resource IDs, etc. Before proceeding further, we are going to cover each of the fundamental data types in KDL, and how they work, as well as some more specialised types.

#### §2.3.1: Integer
Integers are one of the most fundamental types in KDL. They are specified via an integer literal, such as; `10`, `-10` or `0x10`. They are any whole number and can be either positive or negative.

Related to integer literals, are percentage literals. These are simply integer values with a `%` appended to the value. They are provided in order to make the intention of certain resource values clear. For example; `-50%` or `25%`. 

#### §2.3.2: String
String are another one of the fundamental types in KDL, and also specified via a string literal. String literals are any sequence of characters contained between two double quotes, for example; `"This is a string"`

#### §2.3.3: Resource ID / Reference
Resource IDs are the last of the fundamental types in KDL and the last type of literal. Here are some examples of Resource IDs; `#128`, `#-1000`, `#StringList.128`, `#MyFirstMod.StringList.128`

Resource IDs are used to a denote link or relationship with another resource. Historically they have been a simple numeric value ranging between -32,768 and 32,767. Kestrel introduces a new way of representing a link to another resource, through _Resource Refereces_. These are special values that contain multiple components to help Kestrel correctly identify a desired resource.

_**Note:** Resource References will be explained in greater detail in the_ Advanced Concepts _section of the guide._

If the export target of the assembler is a **Rez** or **Classic ResourceFile** then resource references are limited to only their numeric component (i.e. `#128`) and that numeric component must lie within the -32,768 to 32,767 range.

If the export target of the assembler is an **Extended ResourceFile** then the resource references can contain both a _namespace_ and _resource type_, as well as allowing for IDs in the range of approximately -9,223,372,037,000,000,000 to 9,223,372,037,000,000,000.


#### §2.3.4: Color
Colors are complex types that are based on a 32-bit unsigned integer field (`HLNG`) in the resource. They can accept values as an integer directly, or from a function such as `rgb()`.

This will be covered in more detail later.

#### §2.3.5: Bitmask
Bitmasks are complex types that are based on integer types.

This will be covered in more detail later.

#### §2.3.6: Range
Ranges are complex types that are based on either integer types, or numeric resource references. They require that any value supplied to them fall with in the specified range.

#### §2.3.7: File
`File` is an alias of the `String` type allows the user to import the contents of the field from a file.

This will be covered in more detail later.

---
### §2.4: Defining a Resource Type
Let's move on to something move advanced now, creating and defining our own resource types. In this example we are going to define a resource type for describing linear gradients.

```kdl
@type LinearGradient : "lgrd" {
	template {
		PSTR Name;
		HLNG StartingColor;
		HLNG EndingColor;
	};
	
	field("Name") {
		Name;
	};
	
	field("Colors") {
		StartingColor as Color;
		EndingColor as Color;
	};
};
```

There is quite a bit to digest here, so let's step through this in small chunks.

We've already encountered assembler directives, as being instructions to the assembler to perform an action immediately. In the case of the `@type` directive, we are informing the assembler of a new resource type, and that it should accept it as a valid resource type immediately. The directive is followed by a type name and a type code.

Type Names are what is used by KDL and Kestrel to refer to a given resource type, whilst the type code is used to identify a resource type container within a resource file. Type codes are _always_ 4 characters long, regardless of the type of resource file being exported to.

Within the type itself, the first thing we need to do is provide a _template_ to describe the layout of the binary data itself. This is based upon the `TMPL` resource of ResEdit, a tool for editing Resource Forks on the Classic Macintosh. Template fields are specified as pairs of binary data types and field names, i.e: `PSTR Name` and `HLNG StartingColor`.

Following the template, we start defining the fields that the user can/needs to specify in order to populate and instance of a resource for the type. Fields do not need to be a 1:1 mapping to the template, and fields can contain multiple template fields as grouped values.

Let's look at the first field, `Name`. This is about as basic as you can be with a field definition. The definition starts with the `field` keyword, and then is followed by a string representing the field name.

_**Important:** The name is used as an identifier when declaring resources. You must therefore make sure that your name follows the syntactic rules of identifiers. This should be considered a bug, and KDL will check the validity of names in the future._

Inside the field definition we list out the names of each template field that we want to populate when this field is written to. In this case it is the template field `Name`. When the user declares a resource and provides the `Name` it will look something like:

```kdl
	` ...
	Name = "Red";
	` ...
```

In this the `Name` template field, being the first (and only) specified in the field, would be populated by the first value supplied to the field, in this case `"Red"`.

Template fields _must_ be provided in the order that the user is expected to supply the values to the field.

The next field is slightly more complex and interesting. In this example we group two template fields together and specify specialised types for them. In the case of both template fields, we want them to be treated as colors, thus we provide the `as Color` after the template field name. The `Color` specialisation allows for values to be supplied using a convience function:

```kdl
	` ...
	Colors = rgb(255, 0, 0) rgb(0, 0, 255);
	` ...
```

The `rgb(red, green, blue)` function takes three integer arguments (each in the range of `0` - `255`) and combines them into a single integer value `HLNG` representing the color.

This also illustrates how values are supplied to each of the template fields provided in a field. The first value, `rgb(255, 0, 0)` is given to the `StartingColor` template field, whilst the `rgb(0, 0, 255)` is given to the `EndingColor`.

## §3: Managing KDL Projects
Once your project reaches sufficient complexity, you will want to start managing it and organising the data in more efficient ways than massive collections of files and/or thousands of resources grouped under single types.

KDL provides a mechanism for working with large projects, and that is Namespaces/Components.

### §3.1: Namespaces

Namespaces are attached to resource type containers, allowing multiple occurances of a single resource type container to be added to a single file. The benefits of this may not be immediately obvious, but we will explore some of them in this section.

The first method of specifying a namespace is to do it on the resource declaration directly, like so:

```kdl
declare MyNamespace.StringList {
	` ...
}
```

Here we have specified that we should be constructing `StringList` resources within the `MyNamespace` namespace.

### §3.2: Components

The second method of managing namespaces is to use the component construct. This example is going to be a little longer, in order to illustrate how this works and is used.

```kdl
component "Preview" {
	as_type = Picture;
	path_prefix = "images/";
	base_id = #1000;
	namespace = "preview";
	
	files {
		"apple.png" ("Apple");
		"banana.png" ("Banana");
		"grapes.png" ("Grapes");
		"orange.png" ("Orange");
	};
};

component "Thumbnails" {
	as_type = Picture;
	path_prefix = "images/";
	base_id = #1000;
	namespace = "thumbnails";
	
	files {
		"apple-thumbnail.png" ("Apple");
		"banana-thumbnail.png" ("Banana");
		"grapes-thumbnail.png" ("Grapes");
		"orange-thumbnail.png" ("Orange");
	};
};
```

Here we are specifying two components (namespaces). Component constructs are special in KDL as they are used to directly import binary data into a resource. In this case we are importing PNG image data from files on disk into Picture resources.

_**Warning:** This example is not actually correct if using the Picture type in the Macintosh library, and will result in crashes and possibly data corruption._

The two components here are each importing 4 images into the `Picture` type, with the IDs; `#1000`, `#1001`, `#1002` and `#1003`, but in to seperate namespaces; `preview` and `thumbnail`. This has an interesting consequence when accessing the resources in Kestrel. Rather than needing to have calculations to map between resource ID values, related types/resources can be placed in separate namespaces, but with the same ID.

### §3.3: Multiple Files
The final consideration for managing KDL projects, is to not keep all of the KDL itself in a single file, but instead split it across multiple files.

Let's consider a project with the following directory structure.

```
+ MyProject
  |- project.kdl
  |- types.kdl
  |- fruit.kdl
  |- colors.kdl
```

When we invoke the assembler command, we pass it a single file... `project.kdl`, but we also need to be able to use the code located in other files. We can utilize the `@import` directive for this.

```kdl
@import "types.kdl";
@import "fruit.kdl";
@import "colors.kdl";
```

The import directive will load the contents of the specified file, and parse it at the location of the import. Due to the way KDL parses code, if you were to try and utilise a resource type before it is defined inside the imported file, then it will raise an unrecognised resource type error.

By default import directives find the specified file based on the _current working directory_ of the assembler process. KDL provides a few substitution markers for building absolute paths for importing files.

#### §3.3.1: Source Path - `@spath`
Refers to the directory that contains the file in which the current import directive is located. So if we have:

```kdl
` /path/to/project/types/manifest.kdl
@import "@spath/fruit.kdl";
```

In this case `@spath` will resolve to `/path/to/project/types`

#### §3.3.2: Input Path - `@rpath`

Refers to the directory that contains the original input source file is located. So if we have

```sh
> kdl /path/to/project/project.kdl
```

```kdl
` /path/to/project/types/manifest.kdl
@import "@rpath/fruit.kdl";
```

In this case `@rpath` will resolve to `/path/to/project`

#### §3.3.3: Output Path - `@opath`
Refers to the directory where the output file will be located. So if we have:

```sh
> kdl -o /path/to/project/build/result.kdat project.kdl
```

```kdl
` /path/to/project/types/manifest.kdl
@import "@opath/fruit.kdl";
```

In this case `@opath` will resolve to `/path/to/project/build`

## §4: Exporting Types to Kestrel
This section starts to cover some of the more advanced aspects KDL, such as integrating with the scripting functionality in Kestrel. If you are defining new resource types, then it is likely that you want to be able to read and/or write those resource types within a Kestrel based game. KDL provides a method of exporting the type definition and appropriate functionality as a Lua script for use in a Kestrel based game.

### §4.1: Example

Let's consider the `LinearGradient` type definition from earlier:

```kdl
@type LinearGradient : "lgrd" {
	template {
		PSTR Name;
		HLNG StartingColor;
		HLNG EndingColor;
	};
	
	field("Name") {
		Name;
	};
	
	field("Colors") {
		StartingColor as Color;
		EndingColor as Color;
	};
};
```

In order to export this resource type to Lua, we need to make use of the `component` construct. However, we need to need to make a few tweaks to it:

```kdl
` We need to make sure we import Kestrel to get access to the _LuaScript_ type.
@import Kestrel;

@lua_export component "Resource Types" {
	path_prefix = "";
	namespace = "custom_types";
	base_id = #1000;
	as_type = LuaScript;
	
	types {
		LinearGradient;
	};
};
```

This creates a new namespaced component within the `LuaScript` resource type for each of the type definition scripts. In this circumstance we only have a single definition script being produced. The component construct is characterised by the leading `@lua_export` assembler directive, which instructs the assembler to treat the following component differently. Primarily it will no longer take a list of files, but a list of resource types.

Let's take a look at the resulting definition script for `LinearGradient`.

```lua
-- LinearGradient Class Definition
LinearGradient = Class:new()

function LinearGradient.resourceTypeCode()
	return "lgrd"
end

-- Constants
LinearGradient.Constants = {}

-- Construction
function LinearGradient.load(resourceReference)
	if not resourceReference then
		return nil
	end
	
	local resource = LinearGradient:new()
	local data = kdl_resourceLoader(resource, resourceReference, LinearGradient.resourceTypeCode())
	
	if not data then
		return nil
	end
	
	resource._name = data:readPStr()
	resource._startingColor = data:readColor()
	resource._endingColor = data:readColor()
	
	resource._colors = {
		startingColor = self._startingColor
		endingColor = self._endingColor
	}
	
	return resource
end

-- Properties
LinearGradient.properties.name = {}
function LinearGradient.properties.name:get()
	return self._name
end

LinearGradient.properties.colors = {}
function LinearGradient.properties.colors:get()
	return self._colors
end
```

KDL is fully aware of how to generate Lua for Kestrel in order to represent the resource types that you define. However, the Lua that it generates is not always the most clean or logical (primarily due to naming). You could change the names of fields in KDL, but this could introduce the same problem, just within the KDL itself. Instead KDL provides some additional assembler directives that you can add in to your type definitions to inform the Lua code generator what it should do.

Let's take a look at the `Colors` field, after we make some of these alterations.

```kdl
field("Colors") {
	@name(start) StartingColor as Color;
	@name(stop) EndingColor as Color;
};
```

Here we introduce a new assembler directive, the `@name` directive. This is used to instruct the Lua code generator what name should be used for fields/properties when exporting to Lua. It is otherwise ignored by KDL. The consequence of this is that the exported Lua will now look like:

```lua
` ...

resource._colors = {
	start = self._startingColor
	stop = self._endingColor
}

` ...
```

### §4.2: Loading Types in Kestrel
In order to load the generated types into Kestrel, you will need to add the following code to your `Starter` script. Starter scripts, plugins and mods in general are explained in more details in the _§6: Kestrel_ section of the guide.

```lua
-- Starter Script
-- Load custom types.
Namespace("custom_types"):typedResource("LuaS"):matchingResources():each(function(script)
	Kestrel.importScript(script)
end)
```

## §5: Advanced Concepts
We're going start exploring the more advanced concepts of KDL.

### §5.1: Scripting
Since KDL version 0.8, there has been a concerted effort to introduce a degree of scriptability into KDL in order to support the growing requirements and needs of Cosmic Frontier. The two primary aspects of the current scripting capabilities in KDL are _variables_ and _functions_.

#### §5.1.1: Variables
Variables in KDL are defined through either the `@const` or `@var` assembler directives. When defining a variable an expression is evaluated with the result of the expression being assigned to the variable.

```kdl
@const $foo = 5 + 1;
```

Variables can then be used to substitute values in to fields, resource ids, etc.

```kdl
@const $AppleResourceRef = #Fruit.128;
@const $AppleName = "Apple";

declare Fruit {
	new ($AppleResourceRef, $AppleName) {
		Name = $AppleName;
	};
};
```

You can also use those variables in future expressions as well, allowing you to break up complex expressions in to managable parts.

```kdl
@const $foo = 4 * 3;
@const $bar = $foo * 2;
```

_**Important:** All variables are declared into a global scope._

#### §5.1.2: Functions
Functions are defined in a similar way to variable, using the `@function` directive.

```kdl
@function Sum = $1 + $2;
@const $foo = Sum(5, 4);
```

Arguments are not explicity defined on functions, and you should clearly document how many arguments are expected to be passed to a function. You can access the arguments through the indexed variables `$1`, `$2`, `$3`, etc... The result of the function expression becomes the return type. 

Functions can be used in a similar way to variables.

```kdl
@function FruitReference = #Fruit.1000 + $1;
@function FruitPreviewImage = #StaticImage.1000 + $1;

@const $AppleReference = 0;

declare Fruit {
	new (FruitReference($AppleReference), "Apple") {
		Name = "Apple";
		Preview = FruitPreviewImage($AppleReference);
	};
};
```

### §5.2: Default Values
It is possible that some of the values and/or fields you define in your resource type should be optional. In situations like this you need to specify a default value to be used when no value is provided by the user. Default values are simple to implement.

```kdl
field("Name") {
	Name = "Untitled";
};
```


### §5.3: Specialised Data Types
KDL uses specialised data types in order to provide more useful/semantically correct behaviour and functionality to basic binary data types. We explored these briefly in a previous section, but here we'll go into more depth.

#### §5.3.1: File
The `File` type is used to indicate a `String` that is _intended_ to be imported from an external file, such as an image or text.

```kdl
` ...
field("Data") {
	Data as File;
};

`...
Data = import "@spath/description.txt";
```

This is a simple use case of the `File` type, and simply takes the entire contents of the specified file and places them into the specified binary template field.

Getting slightly more advanced, we may want to perform a data conversion on the input data. For instance, we allow the user to import PNG files and then they get encoded into SpriteWorld Sprites. We can use the `__conversion` hint in the field definition to instruct the assembler to convert the format of the data.

```kdl
@import SpriteWorld;

` ...
field("PNG") {
	Data as File<PNG> __conversion($InputFormat, rleD);
};

` ...
Data = import "@spath/sprite.png";
```

A full list of recognised encoding data types are listed in the _§7: API_ section.

In the case of some imports/encodings multiple images might need to be supplied for multiple frames. Sprite image types are one such example. We can supply multiple images like so:

```kdl
Data = import "@spath/frame1.png"
              "@spath/frame2.png"
              "@spath/frame3.png";
```

#### §5.3.2: Resource Referneces
Resource references are a complex subject by themselves as the specialised type can take a few different forms. The most basic of which is:

```kdl
field("Preview") {
	Preview as &;
};
```

Reference types are always represented by having an ampersand `&` suffix, and will only accept resource references as valid values. We can specialise the reference type by doing something such as `Preview as StaticImage&`. Generally it isn't necessarily desirable to provide an explicit reference type like this however, unless you want to enable inline resource declarations.

In order to use a reference type on a binary template field, the binary type needs to be either a signed integer type (`DBYT`, `DWRD`, `DLNG` or `DQAD`) or a resource type (`RSRC`). It should be considered best practice to default to using the `RSRC` binary type to back resource references, as KDL will alter the format appropriately between _Extended_ and _Classic/Rez_ formats.

##### §5.3.2.1: Inline Resource Declaration
If the field value is an explicit reference type, then you can use a inline resource declaration like so:

```kdl
declare Fruit {
	new (#128, "Apple") {
		
		Preview = new (#128, "Apple") {
			PNG = import "@spath/apple.png";
		};
		
	};
};
```

##### §5.3.2.2: Binary Type: RSRC (Classic/Rez)
When the output format of the assembler is either `classic` or `rez`, then the `RSRC` binary type will be treated as a simple `DWRD` type, which allows it to cover all possible resource IDs under those formats.

##### §5.3.2.3: Binary Type: RSRC (Extended)
When the output format of the assembler is `extended` then the `RSRC` binary type will be treated as a complex type. The following definition is for illustrative purposes only.

```
Format:     <FLAGS> <NAMESPACE> <TYPE> <ID>
FLAGS:      8-bits
    0x01    Has Namespace
    0x02    Has Type
	
NAMESPACE:  
    Format: <LENGTH> <BYTES>
    LENGTH: 8-bits
    BYTES:  0-255 MacRoman Characters representing the Namespace
    
TYPE:
    Format: <CODE>
    CODE:   4 MacRoman Characters
	
ID:         64-bits Signed Integer
```

##### §5.3.2.4: Automatic Allocation
It certain situations it is entirely possible that you will not care about what ID a resource is assigned, or that it is consistantly given the same ID. For situations like this, you may wish the special reference `#auto`, which instructs the assembler to automatically allocate an ID to the resource.

The method by which IDs are allocated is not defined, and thus should not be depended upon.

#### §5.3.3: Bitmasks
Bitmasks are a commonly used type of data, typically found in _flags_.

```kdl
field("Flags") {
	Flags as Bitmask [
		Option1 = 0x0001,
		Option2 = 0x0002,
		Option3 = 0x0004,
		Option4 = 0x0008
	];
};

Flags = Option1 | Option4;
```

Whilst it is not mandatory, bitmasks really need a set of constants to be defined alongside them for the user to utilise. Additionally, due to the common pattern of splitting flags between multiple binary template fields, it is possible to "merge" multiple sets of flags together.

```kdl
field("Flags") {
	Flags as Bitmask [
		Option1 = 0x0001,
		Option2 = 0x0002,
		Option3 = 0x0004,
		Option4 = 0x0008
	];
	Flags2 as Bitmask [
		Option5 = 0x0010,
		Option6 = 0x0020,
		Option7 = 0x0040,
		Option8 = 0x0080
	];
};

Flags = Option1 | Option7 | Option3;
```

This allows the user to see all related values such as these as a single field.

The binary template fields backing bitmasks must be an integer type, and it is encouraged that they are also unsigned.

### §5.4: Repeated Fields / Arrays
A common thing to need to represent are repetitions of items, or an Array. There are two common ways of doing this within a resource.

#### §5.4.1: Numbered Template Fields
The first method of providing an array of items is to duplicate the field a number of times and add the field number. Here is an example of such a thing in a binary template:

```kdl
template {
	PSTR Name;
	HLNG Color1;
	DWRD Stop1;
	HLNG Color2;
	DWRD Stop2;
	HLNG Color3;
	DWRD Stop3;
	HLNG Color4;
	DWRD Stop4;
};
```

This first example could be something that we utilise in the `LinearGradient` type from our earlier examples. In this situation with have 4 color fields, representing the colors of the gradient along with their corresponding stop positions.

When defining a field, we can specify the field should be able to be provided multiple times. The first time it is provided would correspond to the first occurance of the field, the second time would then be the second occurance, so on and so forth. Let's take a look at an example field definition for the above template.

```kdl
field("ColorStop") repeatable<1, 4> {
	Color<$FieldNumber> as Color = rgb(0, 0, 0);
	Stop<$FieldNumber> as Range<-1%, 100%> = Unused [ Unused = -1% ];
};
```

There is a little bit to unpack here. Let's start with the first new addition to the field definition; `repeatable<1, 4>`. The repeatable attribute on the field tells the assembler that this field can be repeated between 1 and 4 times. Technically its providing the lower bound and the upper bound on the field numbers, but you can read it as the number of repetitions in this case.

Next is the `<$FieldNumber>` after the binary field name. This entire sequence is substituted by the assembler for the current repetition number. So in the first occurance it would result in `Color1` and `Stop1`, and then in the second it would be `Color2` and `Stop2`.

In order to prevent the assembler from requiring _every_ single repetition to be provided, you must provide default values for each value.

When the user comes to provide the `ColorStop` field, it would look something like:

```kdl
ColorStop = rgb(255, 0, 0) 0%;
ColorStop = rgb(0, 0, 255) 100%;
```

#### §5.4.2: Lists
The other method of providing an array of items is to use a resource list. This is an example of such a binary template (specifically for the `StringList` type)

```kdl
template {
	OCNT Strings;
	LSTC StringsBegin;
	PSTR	String;
	LSTE StringsEnd;
};
```

Unlike the previous method of defining an array, in which each element was explicity declared in the template, lists such as the one in the above example do not provide many fields to be used in the same way. The list is first indicated by the `OCNT` binary type, and each element of the list is wrapped between the `LSTC` and `LSTE` binary types. In this case only a single value exists for each element.

```kdl
field("String") repeatable<0, 32767, Strings> {
	String;
};
```

In this example we find that a 3rd element has been added to the repeatable attribute. This is the name of the list in the binary template that this field is acting upon. The lower and upper bounds can be directly read as how many elements are allowed.

The usage looks identical to the previous example:

```kdl
String = "Foo";
String = "Bar";
String = "Baz";
```

### §5.5: Constants


### §5.6: Lua Code Generation
#### §5.6.1: Writable Properties

### §5.7: Assembler Directives

## §6: Kestrel

## §7: API

### §7.1: Import Libraries
The following libraries are built in to KDL and available for importing.

| Library Name | Purpose |
| --- | --- |
| Macintosh | Provides definitions and functionality related to the Classic Macintosh |
| SpriteWorld | Provides definitions and functionality related to Sprite World |
| Kestrel | Provides definitions and functionality related to the Kestrel Game Engine |

### §7.2 Binary Data Types
The following binary data types are currently recognised by KDL for use in Type Definition Templates.

| Binary Type Name | Minimum Size | Maximum Size | Description |
| --- | --- | --- | --- |
| `DBYT` | 1 | 1 | Signed Byte |
| `DWRD` | 2 | 2 | Signed Word |
| `DLNG` | 4 | 4 | Signed Long |
| `DQAD` | 8 | 8 | Signed Quad |
| `HBYT` | 1 | 1 | Unsigned Byte |
| `HWRD` | 2 | 2 | Unsigned Word |
| `HLNG` | 4 | 4 | Unsigned Long |
| `HQAD` | 8 | 8 | Unsigned Quad |
| `RSRC` (Classic/Rez) | 2 | 2 | Resource Reference (ID) |
| `RSRC` (Extended) | 9 | 269 | Resource Reference (Namespace/TypeCode/ID) |
| `CSTR` | 1 | - | NULL Terminated C String |
| `Cxxx` | xxx + 1 | xxx + 1 | NULL Terminated C String of width `xxx` where `xxx` is hex. |
| `PSTR` | 1 | 256 | Pascal String |
| `HEXD` | 0 | - | Binary Data Blob. Includes all remaining data in resource. |
| `RECT` | 8 | 8 | 4 DWRD values representing Top, Left, Bottom and Right |
| `OCNT` | 2 | 2 | The number of items in the following list minus 1. |
| `LSTC` | 0 | 0 | Denotes the start of the fields to be included in a given list item. |
| `LSTE` | 0 | 0 | Denotes the end of a fields to be included in a given list item. |

