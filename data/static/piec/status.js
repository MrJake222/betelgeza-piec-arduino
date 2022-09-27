const last_good = document.getElementById("last_good_frame")
const status = document.getElementById("last_status")
const frame_stats = document.getElementById("frame_stats")

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
        // console.debug(json)

        let percent = json["wrong_frames"] / (json["good_frames"] + json["wrong_frames"])
        frame_stats.innerHTML = "Błędne ramki: " + Math.round(percent) + "%"
        frame_stats.innerHTML += " (" + json["wrong_frames"] + ")"
    }
}