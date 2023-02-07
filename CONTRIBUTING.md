## Contribution Guidelines

https://www.o3de.org/contribute/#contribution-guidelines

Before you start participating in and contributing to O3DE code such as this Gem, review our [code of conduct](https://o3de.org/docs/contributing/code-of-conduct/) . Contributing guidelines for our projects are hosted in their GitHub repositories, within the root directory in a `CONTRIBUTING.md` file.

## General Contributions:

Contributions are more than just welcome. Fork this repo and create a new branch off of development, then submit a pull request:

1. Fork it, or clone it: https://github.com/o3de/o3de-technicalart
2. Get branches: `git fetch --all`
3. Work off development: `git switch development`
4. Create your feature branch `git checkout -c user_name/my-new-feature`
5. Commit your changes `git commit -s -m 'Add some feature'`
6. Push to the branch `git push -u origin HEAD` (first push, after you can just use `git push`)
7. Create new Pull Request into development

## Guide:

There are several top-level protected branches.  Just like other O3DE repos, the main branch is the stable release branch and is tagged for release, while the development branch is the cutting edge. When working on the code make a branch of development, make your changes, create your PR, run and pass AR, merge into development. o3de-technicalart is not currently Canonical, and does not require Automated Review (AR), although that will be built out over time.

### **Development branches:**

- **Main**: will be used for release cadence. Merging up into main will should require some unit testing and locally run automated test script (in the future the repo will evolve to use AR similar to other o3de repos.)

- **Development**: feature development for supported release work. Merging a PR into development at minimum currently requires multiple maintainers approving code reviews in order to accept the PR, along with some amount of robust manual test plan; some kind of local automated tests are preferred.

- **Prototypes**: PoCs, experimental and prototype work that is not officially supported

Prototypes is where experimental work like research on workflow studies or 'proof of concept' (PoC) work occurs; it is less restrictive but will still require at least one maintainer approving code reviews in order to accept the PR into this branch (this is intended to promote legitimate peer review and collaboration.)  Working in prototypes is similar to the workflow above, but with the following changes

1. Fork it, or clone it: https://github.com/o3de/o3de-technicalart
2. Get branches: `git fetch --all`
3. Work off development: `git switch prototypes`
4. Create your feature branch `git checkout -c user_name/my-new-feature`
5. Commit your changes `git commit -s -m 'Add some feature'`
6. Push to the branch `git push -u origin HEAD` (first push, after you can just use `git push`)
7. Create new Pull Request into prototypes

## O3DE Modularity

The o3de-technicalart repo is similar to the o3de-extras, which is another engine related collection. Gems (are reusable pieces of an o3de project, which other engines might call a plugin) can have other gems as children, and so on. Please refer to the [o3de-extras README](https://github.com/o3de/o3de-extras#readme) for more detailed information.

### **Folder structure:**

1 Gem per-tool.  The gist is that we want a modular structure to support teams enabling the tool integration gems for the DCC tools they specifically use.  Please adhere to an organization such as this:

```
o3de-TechlArt (repo)/
├── Gems/
│   ├── O3DE/
│   │   └── StudioTools
│   ├── DCC/
│   │   ├── Blender
│   │   ├── Autodesk/
│   │   │   ├── Maya
│   │   │   └── ...
│   │   ├── Adobe/
│   │   │   └── Substance3D/
│   │   │       ├── Designer
│   │   │       ├── Painter
│   │   │       └── ...
│   │   └── ...
│   └── IDE/
│       ├── VScode
│       ├── WingPro
│       └── ...
└── Projects/
    ├── DCC_Test_Project
    └── ...
```
