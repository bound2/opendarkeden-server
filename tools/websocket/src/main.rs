//! `gateway [config.json]`: runs the DarkEden WebSocket gateway until ctrl-c
//! or SIGTERM. Diagnostics go to stderr and honour `RUST_LOG` (default
//! `info`; `debug` logs every refused handshake and every connection).

use std::io::Write;
use std::process::ExitCode;

use darkeden_gateway::{shutdown_signal, Config, Gateway};
use tracing_subscriber::EnvFilter;

const USAGE: &str = "usage: gateway [config.json]";

fn main() -> ExitCode {
    tracing_subscriber::fmt()
        .with_env_filter(
            EnvFilter::try_from_default_env().unwrap_or_else(|_| EnvFilter::new("info")),
        )
        .with_writer(std::io::stderr)
        .init();

    let mut args = std::env::args().skip(1);
    let path = match (args.next(), args.next()) {
        (Some(flag), None) if flag == "-h" || flag == "--help" => {
            println!("{USAGE}");
            return ExitCode::SUCCESS;
        }
        (path, None) => path.unwrap_or_else(|| "config.json".to_owned()),
        (_, Some(_)) => {
            eprintln!("{USAGE}");
            return ExitCode::from(2);
        }
    };

    let config = match std::fs::read_to_string(&path)
        .map_err(|error| format!("cannot read {path}: {error}"))
        .and_then(|text| Config::from_json(&text).map_err(|error| format!("{path}: {error}")))
    {
        Ok(config) => config,
        Err(message) => {
            tracing::error!("{message}");
            return ExitCode::FAILURE;
        }
    };

    let runtime = match tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
    {
        Ok(runtime) => runtime,
        Err(error) => {
            tracing::error!("cannot start the async runtime: {error}");
            return ExitCode::FAILURE;
        }
    };

    runtime.block_on(async {
        let gateway = match Gateway::bind(config).await {
            Ok(gateway) => gateway,
            Err(error) => {
                tracing::error!("{error}");
                return ExitCode::FAILURE;
            }
        };
        match gateway.local_addr() {
            Ok(address) => {
                println!("DarkEden gateway listening on {address}");
                let _ = std::io::stdout().flush();
            }
            Err(error) => tracing::warn!("cannot read the bound address: {error}"),
        }
        gateway.serve(shutdown_signal()).await;
        tracing::info!("DarkEden gateway stopped");
        ExitCode::SUCCESS
    })
}
