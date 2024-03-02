            

all_sample_input = {}
all_sample_output = {}

all_input = {}
all_output = {}

use_libs = []

judge_types = {}
problem_status = {}

const pyodidePromise = loadPyodide({
    stdin: stdin_func,
    stdout: stdout_func,
});

function stdin_func() {
    if (submit_run) {
        return all_input[target_objectid][judge_idx];
    } else {
        return all_sample_input[target_objectid];
    }
}

function stdout_func(answer) {
    outputs += answer + "\n";
}

function error_handle(error) {
    document.getElementById(target_objectid + "_out").innerText = error;
    document.getElementById(target_objectid + "_out").style.color = "orange";
    status = "RE";
}

const runCode = async (objectid, require_judge) => {
    target_objectid = objectid;
    submit_run = require_judge;

    let pyodide = await pyodidePromise;

    let editor = ace.edit(objectid);
    let code = editor.getValue();
    let result_bar_id = target_objectid + "_status";
    let result_bar = document.getElementById(result_bar_id)

    document.getElementById(target_objectid + "_out").innerText = "";
    document.getElementById(target_objectid + "_out").style.color = "";
    result_bar.innerText = "Running...";


    if (require_judge) {
        let n_input = all_input[target_objectid].length;
        for (let i = 0; i < n_input; i++) {
            judge_idx = i;
            result_bar.innerText = "Running...   " + judge_idx + "/" + (n_input);
            try {
                outputs = "";
                await pyodide.runPythonAsync(code);
                let expect_out = all_output[target_objectid][judge_idx];
                if (outputs.slice(-1) == "\n") {
                    outputs = outputs.slice(0, -1);
                }
                judge(outputs, expect_out);
            } catch (error) {
                error_handle(error);
                result_bar.innerText = "WJ";
            }

            if (status == "TLE" || status == "MLE" || status == "RE" || status == "WA") {
                result_bar.style.backgroundColor = "#ffe500";
                result_bar.innerText = status + "   " + judge_idx + "/" + (n_input);
                break;
            }
        }

        if (status == "AC") {
            result_bar.style.backgroundColor = "lightgreen";
            result_bar.innerText = "AC";
            confetti();
        } else {
            result_bar.style.backgroundColor = "#ffe500";
        }
    } else {
        try {
            outputs = "";
            await pyodide.runPythonAsync(code);
            document.getElementById(target_objectid + "_out").innerText = outputs;
            result_bar.innerText = "WJ";
        } catch (error) {
            error_handle(error);
            result_bar.innerText = "RE";
        }
    }
};

function judge(answer, expect_out) {
    // trim \n from both
    if (answer.slice(-1) == "\n") {
        answer = answer.slice(0, -1);
    }

    if (expect_out.slice(-1) == "\n") {
        expect_out = expect_out.slice(0, -1);
    }

    let judge_type = judge_types[target_objectid];
    if (judge_type == 'equal') {
        if (answer === expect_out) {
            status = "AC";
        } else {
            status = "WA";
        }
    } else if (judge_type.includes('err')) {
        status = "AC";
        let admissible_absolute_error = parseFloat(judge_type.split('_')[1]);
        let answer_list = answer.split('\n');
        let expect_out_list = expect_out.split('\n');
        for (let i = 0; i < answer_list.length; i++) {
            let answer_line = answer_list[i].split(' ');
            let expect_out_line = expect_out_list[i].split(' ');
            for (let j = 0; j < answer_line.length; j++) {
                let answer_out = parseFloat(answer_line[j]);
                let expect_out_out = parseFloat(expect_out_line[j]);
                if (Math.abs(answer_out - expect_out_out) > admissible_absolute_error) {
                    status = "WA";
                    return;
                }
            }
        }

    }
}

// ExecutableCodeblockのコードを実行する
const runBlock = async (objectid) => {
    let out_area = document.getElementById(objectid + "_out");
    out_area.style.color = "";

    let pyodide = await pyodidePromise;
    let editor = ace.edit(objectid);
    let code = editor.getValue();
    document.pyodideMplTarget = document.getElementById(objectid + "_plot");
    out_area.innerText = "Loading libraries...";
    for (let i = 0; i < use_libs.length; i++) {
        out_area.innerText = "Loading " + use_libs[i] + "...";
        let lib = use_libs[i];
        await pyodide.loadPackage(lib);
    }
    out_area.innerText = "Running...";

    outputs = "";
    try {
        await pyodide.runPythonAsync(code);
        out_area.innerText = outputs;
    } catch (error) {
        out_area.innerText = error;
        out_area.style.color = "orange";
    }
}



