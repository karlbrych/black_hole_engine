// src/main.rs
use anyhow::{Context, Result};
use clap::Parser;
use image::{DynamicImage, GenericImageView, ImageBuffer, imageops};
use std::fs::File;
use std::io::{BufWriter, Write};

#[derive(Parser)]
struct Args {
    /// Input image path (PNG/JPEG)
    input: String,
    /// Output .bhtx path
    output: String,
    /// Minimum mip dimension (stop when either side <= this)
    #[arg(long, default_value_t = 1)]
    min_size: u32,
}

fn main() -> Result<()> {
    let args = Args::parse();
    let img = image::open(&args.input)
        .with_context(|| format!("Failed to open {:?}", args.input))?;

    let (w, h) = img.dimensions();
    let mut levels: Vec<DynamicImage> = Vec::new();
    levels.push(img);

    // Generate mipmaps by repeatedly halving
    loop {
        let last = levels.last().unwrap();
        let (lw, lh) = last.dimensions();
        if lw <= args.min_size && lh <= args.min_size {
            break;
        }
        let nw = (lw / 2).max(1);
        let nh = (lh / 2).max(1);

        let resized = last.resize_exact(nw, nh, imageops::FilterType::Triangle);
        levels.push(resized);
    }

    // Decide channels (we'll always convert to RGBA8 for simplicity)
    let channels = 4u32;

    
    let mut f = BufWriter::new(File::create(&args.output)
        .with_context(|| format!("Failed to create {:?}", args.output))?);

    // Header
    let magic: u32 = 0x42585458; // BXTX
    let version: u32 = 1;
    let mip_levels = levels.len() as u32;

    write_u32(&mut f, magic)?;
    write_u32(&mut f, version)?;
    write_u32(&mut f, w)?;
    write_u32(&mut f, h)?;
    write_u32(&mut f, channels)?;
    write_u32(&mut f, mip_levels)?;

    // Levels
    for lvl in &levels {
        let (lw, lh) = lvl.dimensions();
        let rgba = lvl.to_rgba8();
        let data = rgba.as_raw();

        write_u32(&mut f, lw)?;
        write_u32(&mut f, lh)?;
        write_u32(&mut f, data.len() as u32)?;
        f.write_all(data)?;
    }

    f.flush()?;
    println!("Wrote {:?}", args.output);
    Ok(())
}

fn write_u32<W: Write>(w: &mut W, v: u32) -> Result<()> {
    w.write_all(&v.to_le_bytes())?;
    Ok(())
}
