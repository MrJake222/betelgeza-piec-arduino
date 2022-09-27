async function update() {

    try {
        let resp = await fetch("/proto_get_params")
        if (!resp.ok) {
            throw Error("response mode not ok, error: " + resp.statusText)
        }

        let json = await resp.json()
        // console.debug("Params: ", json)

        update_list.forEach(e => e(json))
    }
    catch (e) {
        console.error("failed to load params")
        console.error("error: " + e)

        const status = document.getElementById("last_status")
        status.innerHTML = "Błąd połączenia"
        status.parentNode.classList.remove("good")
        status.parentNode.classList.add("bad")
    }
}