# Contributing to obs-websocket

## Translating obs-websocket to your language

Localization happens on [Crowdin](https://crowdin.com/project/obs-websocket)

## Branches

**Development happens on `master`**

## Writing code for obs-websocket

### Code Formatting Guidelines

* Function and variable names: camelCase for variables, MixedCaps for method names

* Request and Event names should use MixedCaps names. Keep naming conformity of request naming using similar terms like `Get`, `Set`, `Get[x]List`, `Start[x]`, `Toggle[x]`.

* Request and Event json properties/fields should use camelCase. Try to use existing property names.

* Code is indented with Tabs. Assume they are 4 columns wide

* 80 columns max code width. (Comments/docs can be larger)

* New and updated requests/events must always come with accompanying documentation comments (see existing protocol elements for examples).
These are required to automatically generate the [protocol specification document](docs/generated/protocol.md).

### Code Best-Practices

* Favor return-early code and avoid wrapping huge portions of code in conditionals. As an example, this:
```cpp
if (success) {
    return RequestResult::Success();
} else {
    return RequestResult::Error(RequestStatus::GenericError);
}
```
is better like this:
```cpp
if (!success) {
    return RequestResult::Error(RequestStatus::GenericError);
}
return RequestResult::Success();
```

* Try to use the [built-in](https://github.com/Palakis/obs-websocket/blob/master/src/requesthandler/rpc/Request.h) request checks when possible.
    * Refer to existing requests for usage examples.

* Some example common response/request property names are:
    * `sceneName` - The name of a scene
    * `inputName` - The name of an input
    * `sourceName` - The name of a source (only for when multiple source types apply)
    * `sceneItemEnabled` - Whether a scene item is enabled

* Response parameters which have no attributed data due to an invalid state should be set to `null` (versus being left out)
    * For example, when `GetSceneList` is called and OBS is not in studio mode, `currentPreviewSceneName` will be `null`
    * If a request's core response data depends on a state, an error should be thrown unless `ignoreNonFatalRequestChecks` is set. See `GetCurrentPreviewScene` as an example.

### Commit Guidelines

* Commits follow the 50/72 standard:
    * 50 characters max for the commit title (excluding scope name)
    * One empty line after the title
    * Description wrapped to 72 columns max width per line.

* Commit titles:
    * Use present tense
    * Prefix the title with a "scope" name
        * e.g: "CI: fix wrong behaviour when packaging for OS X"
        * Typical scopes: CI, General, Requests, Events, Server

**Example commit:**

```
Requests: Add GetSceneList

Adds a new request called `GetSceneList` which returns the current
scene, along with an array of objects, each one with a scene name
and index.
```

### Pull Requests

* Pull Requests must never be based off your fork's main branch (in this case, `master`).
    * Start your work in a newly named branch based on the upstream main one (e.g.: `feature/cool-new-feature`, `bugfix/fix-palakis-mistakes`, ...)

* If your work is not done yet, but for any reason you need to PR it (like collecting discussions, testing with CI, getting testers),
    create it as a Draft Pull Request (open the little arrow menu next to the "Create pull request" button, then select "Create draft pull request").