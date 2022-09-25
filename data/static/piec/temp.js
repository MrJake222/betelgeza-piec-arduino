const element_list = [
    {
        element: document.getElementById("co_aktualna"),
        hex_param: "1f5"
    },

    {
        element: document.getElementById("co_zadana"),
        hex_param: "1f6"
    },

    {
        element: document.getElementById("cwu_aktualna"),
        hex_param: "2e6"
    },

    {
        element: document.getElementById("cwu_zadana"),
        hex_param: "28e"
    },
]

function temp_update(json) {
    element_list.forEach(({element, hex_param}) => {

        let val = json[hex_param]
        console.debug(hex_param, val)

        if (val === undefined) {
            element.innerHTML = "--"
        } else {
            element.innerHTML = val
        }
    })
}