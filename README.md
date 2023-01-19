# o3de-technicalart

As the name implies, o3de-technicalart repo (TechArt) is for "Technical Art" related O3DE objects that are considered "Non-Canonical" extensions to O3DE and not "Core" to the engine.  Mainly DCC tool and workflow integrations, and python development environments for technical artists.  We refer to the extensibility of custom content tools and curated workflows as 'Studio Tools'.

## O3DE Modularity

The o3de-technicalart repo is similar to the o3de-extras, which is another engine related repo.  Gems (are reusable pieces of an o3de project, which other engines might call a plugin) can have other gems as children, and so on.  The TechArt repo is a collection of Gems. Please refer to the [o3de-extras README](https://github.com/o3de/o3de-extras#readme) for more detailed information.

## Core, Extras TechArt or somewhere else?

How do we know where to put a new object? If an object's functionality is optional (such as DCC tool integrations), this is a good indication it may not be suitable for the engine core, and more likely should be in the extras or possibly in this repo.

Large objects, such as a large project with binary asset data (3D models, textures, etc) which can be many gigabytes in size, definitely are not in the core. This repo contains one or more such objects in the `\Projects` folder, and each DCC Gem may also contain it's own example and/or test assets.

Extras should be the default place for new development of canonical engine features.  TechArt is suitable for work related to custom tools and workflow development that caters to technical artists and content creation.

## Testing

TechArt is not currently Canonical, and does not require Automated Review (AR), although it will head in that direction over time.  Merging into development at minimum currently requires multiple maintainers approving code reviews in order to accept the PR, along with some amount of robust manual test plan; some kind of local automated tests are preferred.  Merging up into main will additionally require some unit testing and automated test script (in the future the repo will evolve to use AR similar to other O3DE repos.)

## Branches

Just like other O3DE repos, the `main` branch is the stable release branch and is tagged for release, while the `development` branch is the cutting edge. When working on the code make a branch of development, make your changes, create your PR, run and pass AR, merge into development.

Additionally, there is another branch called `prototypes`  is where experimental work like research on workflow studies or 'proof of concept' (PoC) work occurs; it is less restrictive but will still require at least one maintainer approving code reviews in order to accept the PR into this branch (this is intended to promote legitimate peer review and collaboration.)

## Contribute

For more general information about contributing to O3DE, visit [https://o3de.org/docs/contributing/](https://o3de.org/docs/contributing/).  There is also a CONTRIBUTING guide specific to this repo with additional guidance.

## Download and Register o3de-technicalart

### Clone the repository

```shell
git clone https://github.com/o3de/o3de-technicalart.git
```

For more details on setting up the engine, refer to [Setting up O3DE from GitHub](https://o3de.org/docs/welcome-guide/setup/setup-from-github/) in the documentation.

### Setting up o3de-technicalart

Since the TechArt repo can be cloned anywhere on your local computer, we just need to tell O3DE where to find the extra objects in this repo by registering them.  From the O3DE engine repo folder, you can register some or all extra objects using the `o3de register` command.  Since these are all optional objects, we may not need or want all the objects.


If we want to register a particular individual object such *as a single gem* we would issue the following command:

```
scripts\o3de.bat register --gem-path <techart>/Gems/path/<gem name>
```

Or you may want to *register all the Gems*.  Since this repo follows the [standard O3DE compound repo format](https://github.com/o3de/o3de/wiki/O3DE-Standard-repo-formats) all the o3de-technicalart gems will be in the `<techart>/Gems` path. We can therefore register all the gems in the extras gems path with one command:

```
scripts\o3de.bat register --all-gems-path <techart>/Gems
```

This can be repeated for any object type (if they exist):

```
scripts\o3de.bat register --all-projects-path <techart>/Projects
scripts\o3de.bat register --all-gems-path <techart>/Gems
scripts\o3de.bat register --all-templates-path <techart>/Templates
```

If we registered a gem, which is a piece of a project like a plugin, and we want to use that gem in our project we would only have to tell O3DE to enable that gem for our project by using the `o3de enable-gem` command:

```
scripts\o3de.bat enable-gem --gem-name <gem name> --project-name <project name>
```

For a complete tutorial on project configuration, see [Creating Projects Using the Command Line Interface](https://o3de.org/docs/welcome-guide/create/creating-projects-using-cli/) in the documentation.

## License

For terms please see the LICENSE*.TXT files at the root of this distribution.
