import obs from "./obs";

before(async () => {
    await obs.connect({
        password: 'test'
    });
    console.log('Connected');
});

after(() => {
    obs.disconnect();
});
