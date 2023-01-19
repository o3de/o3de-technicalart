## Contribution Guidelines

https://www.o3de.org/contribute/#contribution-guidelines

Before you start participating in and contributing to O3DE code such as this Gem, review our [code of conduct](https://o3de.org/docs/contributing/code-of-conduct/) . Contributing guidelines for our projects are hosted in their GitHub repositories, within the root directory in a `CONTRIBUTING.md` file.

## Contributions:

Contributions are more than just welcome. Fork this repo and create a new branch, then submit a pull request:

1. Fork it: https://github.com/o3de/o3de-technicalart

2. Create your feature branch `git checkout -b my-new-feature`

3. Commit your changes `git commit -am 'Add some feature'`

4. Push to the branch `git push origin my-new-feature`

5. Create new Pull Request

## Guide:

### **Development branches:**

- **main**: will be used for release cadence

- **development**: feature development for supported release work

- **prototypes**: PoCs, experimental and prototype work that is not officially supported

### O3DE Modularity

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
