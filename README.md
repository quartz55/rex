# rex


[![CircleCI](https://circleci.com/gh/yourgithubhandle/rex/tree/master.svg?style=svg)](https://circleci.com/gh/yourgithubhandle/rex/tree/master)


**Contains the following libraries and executables:**

```
rex@0.0.0
│
├─test/
│   name:    TestRex.exe
│   main:    TestRex
│   require: rex.lib
│
├─library/
│   library name: rex.lib
│   namespace:    Rex
│   require:
│
└─executable/
    name:    RexApp.exe
    main:    RexApp
    require: rex.lib
```

## Developing:

```
npm install -g esy
git clone <this-repo>
esy install
esy build
```

## Running Binary:

After building the project, you can run the main binary that is produced.

```
esy x RexApp.exe 
```

## Running Tests:

```
# Runs the "test" command in `package.json`.
esy test
```
