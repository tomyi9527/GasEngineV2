(function()
{
    // editor controller
    let editorController = new mgs.EditorController();
    document.body.appendChild(editorController.getView().getRoot());

    // tick
    let globalClock = new GASEngine.Clock();
    function onTick()
    {
        requestAnimationFrame(onTick);

        let delta = globalClock.getDelta();
        editorController.update(delta);
    };

    onTick();
})();