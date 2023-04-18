# o3de_dreamstudio

A light integration of Stable Diffusion (AI art generation) into an Open 3D Engine Python Gem, by using the Python API for DreamStudio.AI This Gem will create a dream, by sending a prompt to the server, and retrieving the result as an image stored in an asset folder, where they can be used as textures in materials, or for uses such as a digital matte painting backdrop.

What it looks like in Editor
![image (4)](https://user-images.githubusercontent.com/23222931/199061546-8c75d875-d4cb-418e-937d-fa091d6ae67c.png)

Example Dreams
![alien_landscape_basecolor](https://user-images.githubusercontent.com/23222931/199061519-dea0594d-2374-4d4b-86a2-0bf6dc3fc3c5.png)
![default_dream_basecolor](https://user-images.githubusercontent.com/23222931/199061524-3b885779-e95b-4e7f-8028-dcabb72d0596.png)

# To Get Started:

1. Install O3DE, or clone and build the engine from source.
2. Use the O3DE project manager, and create a Project (or use an existing one)
3. Configure Gems for your project, add the DreamStudioAI Gem to your project
4. Build your project then start the Editor.exe

Before use, you will also need to set to use dreamstudio.ai

1. Go to the service and make an account: https://beta.dreamstudio.ai/dream

2. Get your API key: https://beta.dreamstudio.ai/membership?tab=apiKeys

3. In the o3de_dreamstudio gem, make a files called `.secrets.json`

4. Put your API key in and save file:
   
   1. `"STABILITY_API_KEY": "xx-XXxxxxXXxxXxxxXXXXxXXxXxXxXXxXuxxXxXxxXXxxxXXXXx"`

Notes:

- An example secrets file is: `"o3de_dreamstudio\rename.secrets.json.example"`
- You only get 2 sample image generations, buy membership credits for more

## 3rdParty

This O3DE Gem utilizes the stability-sdk for Python, which has other package dependencies that o3de will install during a build.  These are defined in the requirements.txt file in the root of this repo.  Those dependencies and links to their licensing are listed here:

| Package          | License                                                                                                         | Description                                                                                                                                                  |
| ---------------- | --------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| stability-sdk    | [MIT](https://github.com/Stability-AI/stability-sdk/blob/main/LICENSE)                                          | This is the primary package requirement for connecting with the service (remote, or local)                                                                   |
| six              | [MIT](https://github.com/benjaminp/six/blob/master/LICENSE)                                                     | python 2 and 3 compatibility                                                                                                                                 |
| python-magic     | [MIT](https://github.com/ahupp/python-magic/blob/master/LICENSE)                                                | python-magic is a Python interface to the libmagic file type identification library.                                                                         |
| python-magic-bin | [MIT](https://github.com/julian-r/python-magic/blob/master/LICENSE)                                             |                                                                                                                                                              |
| python-dotenv    | [Custom](https://github.com/theskumar/python-dotenv/blob/main/LICENSE)                                          | Python-dotenv reads key-value pairs from a `.env` file and can set them as environment variables.                                                            |
| protobuf         | [Google](https://github.com/protocolbuffers/protobuf/blob/main/LICENSE)                                         | Protocol Buffers (a.k.a., protobuf) are Google's language-neutral, platform-neutral, extensible mechanism for serializing structured data.                   |
| grpcio-tools     | [Apache Software License (Apache License 2.0)](https://pypi.org/project/grpcio-tools/)                          | gRPC is a modern open source high performance Remote Procedure Call (RPC) framework                                                                          |
| grpcio           | [Apache Software License (Apache License 2.0)](https://pypi.org/project/grpcio/)                                |                                                                                                                                                              |
| pillow           | [Historical Permission Notice and Disclaimer (HPND)](https://github.com/python-pillow/Pillow/blob/main/LICENSE) | Pillow, is the friendly PIL fork. PIL is an acronym for Python Imaging Library.  PIL and Pilliow are already python pkgs dependencies of the Open 3D Engine. |

## License

---

Copyright (c) Contributors to the Open 3D Engine Project. For complete
copyright and license terms please see the LICENSE at the root of this
distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT

For terms please see the LICENSE*.TXT files at the root of this distribution.
