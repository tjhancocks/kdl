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

#### $2.3.2: String
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


## §3: Managing KDL Projects

## §4: Exporting Types to Kestrel

## §5: Namespacing

## §6: Advanced Concepts

## §7: Kestrel

## §8: API

### §8.1: Import Libraries
The following libraries are built in to KDL and available for importing.

| Library Name | Purpose |
| --- | --- |
| Macintosh | Provides definitions and functionality related to the Classic Macintosh |
| SpriteWorld | Provides definitions and functionality related to Sprite World |
| Kestrel | Provides definitions and functionality related to the Kestrel Game Engine |

