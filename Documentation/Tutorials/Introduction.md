# Introduction to KDL
This tutorial is intended to give a basic overview of creating plugins and content for the Escape Velocity Nova. Ultimately the KDL tooling will be used for developing content for Kestrel, however given that Kestrel is not yet ready for such use cases, we'll be using EV Nova as our target.

KDL is unlike most plugin editors that have historically been encountered for the Escape Velocity games. It is a text based assembler, taking in _KDL Code_ and assembling it in to a plugin. This approach has a number of advantages to the older method of plugin development:

1. It is easier to divide up sections of your plugin into seperate files and directories.
2. You can use version control systems to manage changes to your code.
3. The workflow is flexible, adapting to how _you_ want to work.

This tutorial is going to assume you have already downloaded and installed KDL, and are comfortable using the command line.

---

## Hello, World
We'll start out with a fairly traditional example... _Hello, World_. This example will serve no real purpose other than to illustrate how we use KDL.

**Example 1:** hello.kdl

```kdl
` A simple Hello, World KDL script.
@out "Hello, World!";
```

This is probably about as simple of a script you can get in KDL, without just having an empty file. This example illustrates two seperate things in KDL, a comment and the `@out` directive.

First of all, comments in KDL are denoted by the backtick character and run from it until the end of the line. This is similar to how comments in many languages work.

The second thing to examine here is the concept of a _directive_. Directives are instructions to the KDL assembler itself to do something. They are always prefixed by the `@` character, and thus easily identifiable. These instructions might be to import the contents of another file as additional KDL code, or to define a new resource type. 

In this case the directive in use is `@out`. The `@out` directive is used to print a message to the assemblers standard output.

The final thing to note about KDL is that **all** commands must be terminated by a semi-colon `;`.

Let's try assembling this script. For this you will need to use your favorite/preferred terminal, and navigate to the directory in which you have saved the `hello.kdl` file.

**Example 1.1:** Assembling

```sh
$ kdl hello.kdl
Hello, World!
```

Enter in the command shown above, and you'll see the message "Hello, World!" printed out in the terminal. As KDL assembled the script, in encountered the `@out` directive and printed the message to its output.

Let's get on to something more interesting though!

## Creating Resources
So what we did before is useful for understanding how KDL is used, but no particularly useful for much else. 

So let's create a simple plugin!

We'll make a small plugin to add an additional character type to the game. Let's start by creating a new directory for our new plugin. I've created a directory at `~/Documents/AuroranPilot`.

As the name of the directory implies, our plugin will introduce a new character template into the game that has us start our journey in the heart of the Auroran empire, rather than in Federation space.

Let's get started...

**Example 2:** ~/Documents/AuroranPilot/Character.kdl

```kdl
declare PlayerCharacterTemplate {
	new (#129, "Auroran Pilot") {
		` ...
	};
};
```

Before we continue lets take a look at what we have above and what it all means or represents.

The `declare` keyword is used to indicate that we are going to be declaring new resources of a given type. The type is specified immediately following the `declare` keyword. In this case the type is `PlayerCharacterTemplate`. KDL knows from the EV Nova type definitions that the `PlayerCharacterTemplate` type should be assembled into `chär` resource.

All of the resources being declared need to be encapsulated within the braces `{` ... `}` as seen above. The closing brace should be followed by a semi-colon.

Each individual resource is then denoted via the `new` keyword, and must be given an ID and optionally a name.

Resource ID's are always denoted using the `#` character, like so: `#128`, `#23548`, `#-23`, etc... Once again this is to make the usage of a resource id explicit to reader. If the number has a leading `#` then it is a resource id.

The resource name does not need to be specified for a resource. If it is specified then the name is expected to be a string. Strings in KDL are denoted through the `"` double quote characters.

The content of the resource is specified through fields, like so:

```kdl
FieldName = Value;
```

On the left side of the equals `=` is the name of the field and on the right is the value. The value might be one of a number of different types, such as an integer, resource id, string or a symbol. We'll look at these types in greater detail later on.

**Example 2.1:** ~/Documents/AuroranPilot/Character.kdl

```kdl
declare PlayerCharacterTemplate {
	new (#129, "Auroran Pilot") {
		Cash = 25000;
	};
};
```

So far we've only added a single field into the resource, the amount of cash that the player will start with. The cash field on a `PlayerCharacterTemplate` is defined as being an integer.

Once again, note the semi-colon at the end of the line.

**Example 2.2:** ~/Documents/AuroranPilot/Character.kdl

```kdl
declare PlayerCharacterTemplate {
	new (#129, "Auroran Pilot") {
		Cash = 25000;
		Ship = #152;
	};
};
```

Now we've added another field to the resource... the starting ship. In this case the field has been provided a resource id as its value. Once again the `PlayerCharacterTemplate` defines the field as requiring a resource ID for a `Ship`.

**Example 2.3:** ~/Documents/AuroranPilot/Character.kdl

```kdl
declare PlayerCharacterTemplate {
	new (#129, "Auroran Pilot") {
		Cash = 25000;
		Ship = #152;
		System = #315 #298 #311;
	};
};
```

In this example we encounter something slightly different. The starting system field has _3_ seperate values assigned to it. The `chär` resource type as seen by EV Nova has 4 seperate fields `System1`, `System2`, `System3` and `System4` for defining the potential starting systems. KDL offers the ability to condense these down in to a single field. The `System` field can take up to 4 different values and will automatically substitute missing values with a default value `#-1`, which EV Nova will pick up as meaning unused.

Let's continue on and fill out the rest of the resource.

**Example 2.4:** ~/Documents/AuroranPilot/Character.kdl

```kdl
declare PlayerCharacterTemplate {
	new (#129, "Auroran Pilot") {
		Cash = 25000;
		Ship = #152;
		System = #315 #298 #311;
		Kils = 0;
		AddIntroSequenceFrame = #8200 45;
		AddIntroSequenceFrame = #8201 45;
		AddIntroSequenceFrame = #8203 45;
		Flags = 0;
		StartingDate = 23 6 1177;
		DateSuffix = "NC";
	};
};
```

There is one new type of field to mention here. The `AddIntroSequenceFrame` is repreated 3 times. Each instance of the field represents a picture to be shown to the player and the duration in which it should be shown, before the game begins. These "repeatable" fields help to cut down on the amount of code being written, and to make field naming less tedious.

Now that we have our charater template declared, let's build our plugin.

Once again we need to be in the terminal, and navigated to the appropriate directory.

**Example 2.5:** Assembling

```sh
$ cd ~/Documents/AuroranPilot
$ kdl --scenario evn -o AuroranPilot Character.kdl
```

You may have noticed a few extra options being present in the command. These are:

- `--scenario evn`: This tells KDL what game scenario we are assembling a plugin for. By default KDL only comes with the `evn` (EV Nova) scenario type definitions.
- `-o AuroranPilot`: This tells KDL where the output of the assembly should be saved. In this case the `AuroranPilot` file. KDL will automatically determine what extension is appropriate.

And there you have it. You have built a simple plugin using KDL. From here on, the process of adding the plugin to EV Nova is just the same as it has always been.