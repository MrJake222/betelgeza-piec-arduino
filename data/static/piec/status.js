const last_good = document.getElementById("last_good_frame")
const status = document.getElementById("last_status")

function status_update(json) {
    if (json["last_status"] == "") {
        status.innerHTML = "Brak ramek"
        status.parentNode.classList.remove("good")
        status.parentNode.classList.add("bad")
    } else {
        status.innerHTML = json["last_status"]

        let err = json["last_status"].startsWith("ERR")

        if (err) {
            status.parentNode.classList.remove("good")
            status.parentNode.classList.add("bad")
        } else {
            status.parentNode.classList.remove("bad")
            status.parentNode.classList.add("good")
        }

        last_good.innerHTML = json["last_good_frame"]
    }
}